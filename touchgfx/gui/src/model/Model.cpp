#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "TelescopeBackend.h"

static EquatorialCoordinates eq_coord;
static MountCoordinates mount_coord;
static int timezone;

Model::Model() :
		modelListener(0)
{
	TelescopeBackend::initialize();
	TelescopeBackend::syncTime();
}

void Model::tick()
{
	static int timeupdate = 0;
	static int timesync = 0;
	if (modelListener)
	{
		if (timeupdate-- == 0)
		{
			// Update time every 10 ticks
			modelListener->setTime(time(NULL), timezone);
			EquatorialCoordinates new_coords;
			MountCoordinates new_coords_mount;
			if (TelescopeBackend::getEqCoords(new_coords) == 0)
			{
				eq_coord = new_coords;
			}
			if (TelescopeBackend::getMountCoords(new_coords_mount) == 0)
			{
				mount_coord = new_coords_mount;
			}

			modelListener->setCoords(eq_coord, mount_coord);
			timeupdate = 10;
		}

		if (timesync-- == 0)
		{
			TelescopeBackend::syncTime();
			timezone = TelescopeBackend::getConfigInt("timezone");
			timesync = 100;
		}

	}
}

time_t Model::getTime()
{
	return time(NULL);
}

EquatorialCoordinates Model::getEqCoords()
{
	return eq_coord;
}

MountCoordinates Model::getMountCoords()
{
	return mount_coord;
}

int Model::getTimeZone()
{
	return timezone;
}
