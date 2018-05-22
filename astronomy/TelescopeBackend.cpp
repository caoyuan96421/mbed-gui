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
static const int TIMEOUT_IMMEDIATE = 500; // Commands that should immediately return
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

static void read_thread()
{
	char buf[1024];
	char linebuf[1024];
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
	{
		len = snprintf(buf, sizeof(buf), "%s %s\r\n", command, arg);

		debug_if(TB_DEBUG, "commandSent: %s+%s=%s\r\n", command, arg, buf);
	}

// Prepare command tracking structure
	ListNode *cmd_node = commandStarted(command, timeout);

// Write command
	serial.write(buf, len);

	return cmd_node;
}

static bool queryHasReturned(ListNode *p)
{
	return p->retval != 0x7FFFFFFF;
}

static int queryMessage(ListNode *p, char *buf, int size, int timeout)
{
	if (!p || queryHasReturned(p))
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
	if (ret && TB_DEBUG)
	{
		error("ret=%d", ret);
	}
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
	debug_if(TB_DEBUG, "Setting time to %d\r\n", timestamp);
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
	debug_if(TB_DEBUG, "RA=%f, DEC=%f\r\n", ra, dec);
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
	debug_if(TB_DEBUG, "RA=%f, DEC=%f\r\n", ra, dec);
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

int TelescopeBackend::track(bool on)
{
	if (on)
		queryNoResponse("track", NULL);
	else
		queryNoResponse("stop", "track");
	return 0;
}

TelescopeBackend::mountstatus_t TelescopeBackend::getStatus()
{
	char buf[32];
	ListNode *node = queryStart("status", NULL, TIMEOUT_IMMEDIATE);
	if (!node)
	{
		return UNDEFINED;
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

	debug("state: %s\r\n", buf);

	if (strcmp(buf, "stopped") == 0)
		return MOUNT_STOPPED;
	else if (strcmp(buf, "slewing") == 0)
		return MOUNT_SLEWING;
	else if (strcmp(buf, "tracking") == 0)
		return MOUNT_TRACKING;
	else if (strcmp(buf, "nudging") == 0)
		return MOUNT_NUDGING;
	else if (strcmp(buf, "nudging_tracking") == 0)
		return MOUNT_NUDGING_TRACKING;
	else
		return UNDEFINED;

	failed: debug("Failed to read status.\r\n");
	queryFinish(node);
	return UNDEFINED;
}

static void setDataValue(DataType type, char *str, DataValue &value)
{
	switch (type)
	{
	case DATATYPE_STRING:
		strncpy(value.strdata, str, sizeof(value.strdata));
		return;
	case DATATYPE_INT:
		value.idata = strtol(str, NULL, 10);
		return;
	case DATATYPE_DOUBLE:
		value.ddata = strtod(str, NULL);
		return;
	case DATATYPE_BOOL:
		value.bdata = (strcmp(str, "true") == 0);
		return;
	}
}

int TelescopeBackend::getConfigAll(ConfigItem* configs, int maxConfig)
{
	char buf[1024];
	int nConfig = 0;
	char *saveptr;
	const char *delim = " ";
	char *str = buf;

// Read all configs
	ListNode *node = queryStart("config", NULL, TIMEOUT_IMMEDIATE);
	if (!node)
	{
		return 0;
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

	for (nConfig = 0; nConfig < maxConfig; nConfig++)
	{
		char *config = strtok_r(str, delim, &saveptr);
		if (config == NULL)
		{
			// End of string
			break;
		}
		configs[nConfig].setConfig(config);
		str = NULL;
	}

// Read name, type and help
	char query[64];
	delim = ",";
	for (int i = 0; i < nConfig; i++)
	{
		snprintf(query, sizeof(query), "%s info", configs[i].config);
		node = queryStart("config", query, TIMEOUT_IMMEDIATE);
		if (!node)
		{
			return 0;
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
		// Get type
		char *type = strtok_r(buf, delim, &saveptr);
		if (strcmp(type, "INT") == 0)
		{
			configs[i].type = DATATYPE_INT;
		}
		else if (strcmp(type, "DOUBLE") == 0)
		{
			configs[i].type = DATATYPE_DOUBLE;
		}
		else if (strcmp(type, "BOOL") == 0)
		{
			configs[i].type = DATATYPE_BOOL;
		}
		else if (strcmp(type, "STRING") == 0)
		{
			configs[i].type = DATATYPE_STRING;
		}
		else
		{
			goto failed;
		}
		// Get value
		char *value = strtok_r(NULL, delim, &saveptr);
		setDataValue(configs[i].type, value, configs[i].value);
		// Get name
		char *name = strtok_r(NULL, delim, &saveptr);
		configs[i].setName(name);
		// Get help from the rest
		configs[i].setHelp(saveptr);

		// TODO get limits
	}

	return nConfig;

	failed: debug("Failed to read config.\r\n");
	queryFinish(node);
	return nConfig;
}

void TelescopeBackend::setSpeed(const char *type, double speed)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%s %.8f", type, speed);
	queryNoResponse("speed", buf);
}

double TelescopeBackend::getSpeed(const char* type)
{
	char buf[32];
	ListNode *node = queryStart("speed", type, TIMEOUT_IMMEDIATE);
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

	return strtod(buf, NULL);

	failed: debug("Failed to read speed %s.\r\n", type);
	queryFinish(node);
	return NAN;
}
