/*
 * TelescopeBackend.cpp
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#include <TelescopeBackend.h>
#include <cstdio>

UARTSerial serial(D1, D0, 38400);

// Timeout values for different commands
static const int TIMEOUT_IMMEDIATE = 50; // Commands that should immediately return

static int send_command(const char *command, const char *arg, char *retval, int maxsize, int timeout)
{
	if (!retval)
		return -1;
	char buf[256];
	int len;

	// First flush the output buffer
	serial.set_blocking(false);
	while (serial.read(buf, sizeof(buf)) >= 0)
		;

	if (!arg)
		len = snprintf(buf, sizeof(buf), "%s\r\n", command);
	else
		len = snprintf(buf, sizeof(buf), "%s %s\r\n", command, arg);

	// Write command
	serial.set_blocking(true);
	serial.write(buf, len);

	// Get result
	Timer t;
	t.start();

	char *p = retval;
	char *g = retval + maxsize;
	char *l = retval;
	char cmd[32];
	int status;

	serial.set_blocking(false);

	while (t.read_ms() < timeout)
	{
		// Wait for something
		len = serial.read(buf, sizeof(buf));
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
				// Copy content to the return buffer if there is space
				if (p < g)
				{
					*(p++) = '\n';
				}
				// Check if it is status return from the command
				sscanf(l, "%s%d", cmd, &status);
				if (strcmp(cmd, command) == 0)
				{
					return status;
				}

				l = p;
				break;
			default:
				if (p < g)
					*(p++) = buf[i];
				break;
			}
		}
	}

	return -1;
}

int TelescopeBackend::syncTime()
{
	char buf[32];
	int ret = send_command("time", "stamp", buf, sizeof(buf), TIMEOUT_IMMEDIATE);
	if (ret)
	{
		debug("Failed to sync time.\r\n");
		return ret;
	}
	time_t timestamp = strtol(buf, NULL, 10);
	debug("Setting time to %d\r\n", timestamp);
	set_time(timestamp);
	return 0;
}

EquatorialCoordinates TelescopeBackend::getEqCoords()
{
	char buf[64];
	int ret = send_command("read", NULL, buf, sizeof(buf), TIMEOUT_IMMEDIATE);
	if (ret)
	{
		debug("Failed to read coordinates.\r\n");
		return ret;
	}
	double ra, dec;
	if (sscanf(buf, "%lf%lf", &ra, &dec) != 2)
	{
		debug("Failed to read coordinates.\r\n");
		return -1;
	}
	debug("RA=%f, DEC=%f\r\n", ra, dec);
	return EquatorialCoordinates(dec, ra);
}
