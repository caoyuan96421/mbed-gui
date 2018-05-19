/*
 * TelescopeBackend.cpp
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#include <TelescopeBackend.h>
#include <cstdio>
#include <cctype>
#include "mbed.h"

#define TB_DEBUG 0

UARTSerial serial(D1, D0, 76800);

// Timeout values for different commands
static const int TIMEOUT_IMMEDIATE = 50; // Commands that should immediately return
static Mutex mutex;
static Thread backend_thread(osPriorityBelowNormal, OS_STACK_SIZE, NULL, "backend_thread");
static Timer timer;

struct ListNode
{
	const char *cmd;
	uint32_t time_deadline;
	ListNode *next;
	ListNode *prev;
	Queue<char, 64> msgqueue;
	Semaphore commandReturned;
	int retval;
	ListNode() :
			cmd(NULL), time_deadline(0), next(NULL), prev(NULL), commandReturned(0), retval(0x7FFFFFFF)
	{
	}
};

static ListNode listhead;

static ListNode *commandStarted(const char *cmd, int timeout)
{
	debug_if(TB_DEBUG, "commandStarted: %s\r\n", cmd);
	mutex.lock();
	ListNode *p, *q;
	for (p = &listhead; p->next; p = p->next)
		;
	q = new ListNode;
	if (!q)
	{
		mutex.unlock();
		return NULL;
	}
	q->cmd = cmd;
	q->time_deadline = timer.read_ms() + timeout;
	q->next = NULL;
	p->next = q;
	q->prev = p;
	mutex.unlock();
	return q;
}

static ListNode *findCommand(const char *cmd)
{
	ListNode *p;
	for (p = listhead.next; p && strcmp(p->cmd, cmd) != 0; p = p->next)
		;
	return p;
}

static int addMessage(const char *cmd, char *str, int size)
{
	mutex.lock();
	ListNode *p = findCommand(cmd);
	if (!p)
	{
		mutex.unlock();
		return -1;
	}
	debug_if(TB_DEBUG, "addMessage: %s (%s)\r\n", str, p->cmd);
	if (p->msgqueue.full())
	{
		mutex.unlock();
		return -1;
	}
	char *msg = new char[size + 1];
	if (!msg)
	{
		mutex.unlock();
		return -1;
	}
	strncpy(msg, str, size);
	msg[size] = '\0'; // Ensure proper termination
	p->msgqueue.put(msg, 0, 0);
	mutex.unlock();
	return 0;
}

static void nodeDelete(ListNode *node)
{
	if (node)
	{
		// Delete all messages
		debug_if(TB_DEBUG, "nodeDelete: %s\r\n", node->cmd);
		mutex.lock();
		while (!node->msgqueue.empty())
		{
			delete (char*) node->msgqueue.get().value.p;
		}
		if (node->prev)
		{
			node->prev->next = node->next;
		}
		if (node->next)
		{
			node->next->prev = node->prev;
		}
		delete node;
		mutex.unlock();
	}
}

static char linebuf[512];

static void read_thread()
{
	char buf[128];
	char *p = linebuf;
	char *g = linebuf + sizeof(linebuf);
	char cmd[32];
	ListNode *node;
	while (true)
	{
		int len = serial.read(buf, sizeof(buf));
		if (len < 0)
		{
			Thread::wait(1);
			continue;
		}
		for (int i = 0; i < len; i++)
		{
			switch (buf[i])
			{
			case '\r':
				break;
			case '\n':
				if (p < g)
				{
					*(p++) = '\0';
				}
				else
				{
					*(--p) = '\0';
				}

				debug_if(TB_DEBUG, "|%s|\r\n", linebuf);
				// Check if it is status return from the command
				if (isdigit(linebuf[0]))
				{
					int retval;
					sscanf(linebuf, "%d %s", &retval, cmd);

					// Send a signal to whoever waiting for the command to finish
					mutex.lock();
					if ((node = findCommand(cmd)) != NULL)
					{
						node->retval = retval;
						node->commandReturned.release();
					}
					mutex.unlock();
				}
				else
				{
					char *saveptr;
					const char *delim = " ";
					strncpy(cmd, strtok_r(linebuf, delim, &saveptr), sizeof(cmd));
					cmd[sizeof(cmd) - 1] = '\0';

					addMessage(cmd, saveptr, strlen(saveptr));
				}

				p = linebuf; // Rewind
				break;
			default:
				if (p < g)
					*(p++) = buf[i];
				break;
			}
		}
	}
}

static void queryNoResponse(const char *command, const char *arg)
{
	char buf[256];
	int len;

	// Make command
	if (!arg)
		len = snprintf(buf, sizeof(buf), "%s\r\n", command);
	else
		len = snprintf(buf, sizeof(buf), "%s %s\r\n", command, arg);

	// Write command, ignore output
	serial.write(buf, len);
}

static ListNode *queryStart(const char *command, const char *arg, int timeout)
{
	char buf[256];
	int len;

	if (!arg)
		len = snprintf(buf, sizeof(buf), "%s\r\n", command);
	else
		len = snprintf(buf, sizeof(buf), "%s %s\r\n", command, arg);

	// Prepare command tracking structure
	ListNode *cmd_node = commandStarted(command, timeout);

	// Write command
	serial.write(buf, len);

	return cmd_node;
}

static int queryMessage(ListNode *p, char *buf, int size, int timeout)
{
	if (!p)
	{
		return -1;
	}
	osEvent evt = p->msgqueue.get(timeout);
	if (evt.status != osEventMessage)
	{
		return -2;
	}
	char *msg = (char*) evt.value.p;
	strncpy(buf, msg, size);
	delete msg;
	return 0;
}

static bool queryHasReturned(ListNode *p)
{
	return p->retval != 0x7FFFFFFF;
}

static int queryWaitForReturn(ListNode *p, int timeout)
{
	if (!p)
	{
		return -1;
	}
	if (p->commandReturned.wait(timeout) <= 0)
	{
		return -2;
	}
	int ret = p->retval;
	return ret;
}

static void queryFinish(ListNode *p)
{
	nodeDelete(p);
}

void TelescopeBackend::initialize()
{
	timer.start();
	backend_thread.start(read_thread);
}

int TelescopeBackend::syncTime()
{
	char buf[32];
	time_t timestamp;
	ListNode *node = queryStart("time", "stamp", TIMEOUT_IMMEDIATE);
	if (!node)
	{
		return -1;
	}
	if (queryMessage(node, buf, sizeof(buf), TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	if (queryWaitForReturn(node, TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	queryFinish(node);
	timestamp = strtol(buf, NULL, 10);
	debug("Setting time to %d\r\n", timestamp);
	set_time(timestamp);
	return 0;

	failed: debug("Failed to sync time.\r\n");
	queryFinish(node);
	return -1;
}

int TelescopeBackend::getEqCoords(EquatorialCoordinates &out)
{
	char buf[64];
	double ra, dec;
	ListNode *node = queryStart("read", NULL, TIMEOUT_IMMEDIATE);
	if (!node)
	{
		return -1;
	}
	if (queryMessage(node, buf, sizeof(buf), TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	if (queryWaitForReturn(node, TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	queryFinish(node);

	if (sscanf(buf, "%lf%lf", &ra, &dec) != 2)
	{
		goto failed;
	}
	debug("RA=%f, DEC=%f\r\n", ra, dec);
	out = EquatorialCoordinates(dec, ra);
	return 0;

	failed: debug("Failed to read coordinates.\r\n");
	queryFinish(node);
	return -1;
}

int TelescopeBackend::getMountCoords(MountCoordinates &out)
{
	char buf[64];
	double ra, dec;
	ListNode *node = queryStart("read", "mount", TIMEOUT_IMMEDIATE);
	if (!node)
	{
		return -1;
	}
	if (queryMessage(node, buf, sizeof(buf), TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	if (queryWaitForReturn(node, TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	queryFinish(node);

	if (sscanf(buf, "%lf%lf", &ra, &dec) != 2)
	{
		goto failed;
	}
	debug("RA=%f, DEC=%f\r\n", ra, dec);
	out = MountCoordinates(dec, ra);
	return 0;

	failed: debug("Failed to read coordinates.\r\n");
	queryFinish(node);
	return -1;
}

int TelescopeBackend::startNudge(Direction dir)
{
	const char *dir_str;
	switch (dir)
	{
	case EAST:
		dir_str = "east";
		break;
	case WEST:
		dir_str = "west";
		break;
	case SOUTH:
		dir_str = "south";
		break;
	case NORTH:
		dir_str = "north";
		break;
	}
	queryNoResponse("nudge", dir_str);
	return 0;
}

int TelescopeBackend::stopNudge()
{
	queryNoResponse("nudge", "stop");
	return 0;
}

void TelescopeBackend::emergencyStop()
{
	queryNoResponse("estop", NULL);
}

int TelescopeBackend::getConfigString(const char* config, char* buf, int size)
{
	ListNode *node = queryStart("config", config, TIMEOUT_IMMEDIATE);
	if (!node)
	{
		return -1;
	}
	if (queryMessage(node, buf, size, TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	if (queryWaitForReturn(node, TIMEOUT_IMMEDIATE) != 0)
	{
		goto failed;
	}
	queryFinish(node);

	return 0;

	failed: debug("Failed to read config.%s.\r\n", config);
	queryFinish(node);
	return -1;
}

int TelescopeBackend::getConfigInt(const char* config)
{
	char buf[64];
	getConfigString(config, buf, sizeof(buf));
	return strtol(buf, NULL, 10);
}

double TelescopeBackend::getConfigDouble(const char* config)
{
	char buf[64];
	getConfigString(config, buf, sizeof(buf));
	return strtod(buf, NULL);
}

bool TelescopeBackend::getConfigBool(const char* config)
{
	char buf[64];
	getConfigString(config, buf, sizeof(buf));
	return (strcmp(config, "true") == 0);
}
