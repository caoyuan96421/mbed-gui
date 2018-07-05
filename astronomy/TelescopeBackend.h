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
	char *config;
	char *name;
	char *help;
	DataType type;
	DataValue value;
	DataValue min;
	DataValue max;

	ConfigItem() :
			config(NULL), name(NULL), help(NULL), type(DATATYPE_INT)
	{
	}
	~ConfigItem()
	{
		if (config)
			delete[] config;
		if (name)
			delete[] name;
		if (help)
			delete[] help;
	}

	void setName(const char *n)
	{
		if (name)
			delete[] name;
		name = new char[strlen(n) + 1];
		if (name)
		{
			strcpy(name, n);
		}
	}
	void setConfig(const char *c)
	{
		if (config)
			delete[] config;
		config = new char[strlen(c) + 1];
		if (config)
		{
			strcpy(config, c);
		}
	}
	void setHelp(const char *h)
	{
		if (help)
			delete[] help;
		help = new char[strlen(h) + 1];
		if (help)
		{
			strcpy(help, h);
		}
	}

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
	typedef enum
	{
		UNDEFINED = -1, MOUNT_STOPPED = 0, MOUNT_SLEWING = 1, MOUNT_TRACKING = 2, MOUNT_NUDGING = 4, MOUNT_NUDGING_TRACKING = MOUNT_TRACKING | MOUNT_NUDGING
	} mountstatus_t;

	static void initialize();

	static int syncTime();
	static int getEqCoords(EquatorialCoordinates &);
	static int getMountCoords(MountCoordinates &);

	static int getConfigAll(ConfigItem *, int);
	static int getConfigString(const char *config, char *buf, int size);
	static int getConfigInt(const char *config);
	static double getConfigDouble(const char *config);
	static bool getConfigBool(const char *config);
	static void writeConfig(ConfigItem *);

	static int startNudge(Direction dir);
	static int stopNudge();
	static void emergencyStop();

	static int goTo(EquatorialCoordinates eq);
	static int goToMount(MountCoordinates eq);

	static int getCalibration(EqCalibration &calib);
	static int getNumAlignmentStars();
	static int getAlignmentStar(int index, AlignmentStar &star);
	static int addAlignmentStar(const AlignmentStar &star);
	static int addAlignmentStar(const EquatorialCoordinates &star_ref);
	static void removeAlignmentStar(int index);
	static int replaceAlignmentStar(int index, const AlignmentStar &star);
	static int replaceAlignmentStar(int index, const EquatorialCoordinates &star_ref);
	static void clearAlignment();
	static EquatorialCoordinates convertMountToEquatorial(const MountCoordinates &mc);
	static MountCoordinates convertEquatorialToMount(const EquatorialCoordinates &eq);

	static int track(bool on);
	static mountstatus_t getStatus();

	static double getSpeed(const char *type);

	static void setSpeed(const char *type, double speedSidereal);

private:
	TelescopeBackend();
	~TelescopeBackend();
};

#endif /* TELESCOPEBACKEND_H_ */
