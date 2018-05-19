/*
 * TelescopeBackend.h
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPEBACKEND_H_
#define TELESCOPEBACKEND_H_

#include "mbed.h"
#include "CelestialMath.h"

typedef enum
{
	DATATYPE_INT, DATATYPE_DOUBLE, DATATYPE_STRING, DATATYPE_BOOL
} DataType;

typedef union
{
	int idata;
	double ddata;bool bdata;
	char strdata[32];
} DataValue;

struct ConfigItem
{
	const char *name;
	const char *help;
	DataType type;
	DataValue value;
	DataValue min;
	DataValue max;
};

class TelescopeBackend
{
public:
	typedef enum
	{
		NORTH, ///< An enum constant representing the north option
		SOUTH, ///< An enum constant representing the south option
		EAST,  ///< An enum constant representing the east option
		WEST   ///< An enum constant representing the west option
	} Direction;

	static void initialize();

	static int syncTime();
	static int getEqCoords(EquatorialCoordinates &);
	static int getMountCoords(MountCoordinates &);

	static int getConfigString(const char *config, char *buf, int size);
	static int getConfigInt(const char *config);
	static double getConfigDouble(const char *config);
	static bool getConfigBool(const char *config);

	static int startNudge(Direction dir);
	static int stopNudge();
	static void emergencyStop();

private:
	TelescopeBackend();
	~TelescopeBackend();
};

#endif /* TELESCOPEBACKEND_H_ */
