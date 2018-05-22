#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() :
		modelListener(0)
{
	TelescopeBackend::initialize();
	syncImm = true;
	king = false;
	timezone = 0;
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

		if (timesync-- == 0 || syncImm)
		{
			syncImm = false;
			TelescopeBackend::syncTime();
			timezone = TelescopeBackend::getConfigInt("timezone");
			location = LocationCoordinates(TelescopeBackend::getConfigDouble("latitude"), TelescopeBackend::getConfigDouble("longitude"));
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

LocationCoordinates Model::getLocation()
{
	return location;
}

TelescopeBackend::mountstatus_t Model::getStatus()
{
	return TelescopeBackend::getStatus();
}

void Model::track(bool on)
{
	TelescopeBackend::track(on);
}

int Model::getConfigString(const char* config, char* buf, int size)
{
	return TelescopeBackend::getConfigString(config, buf, size);
}

int Model::getConfigInt(const char* config)
{
	return TelescopeBackend::getConfigInt(config);
}

double Model::getConfigDouble(const char* config)
{
	return TelescopeBackend::getConfigDouble(config);
}

bool Model::getConfigBool(const char* config)
{
	return TelescopeBackend::getConfigBool(config);
}

double Model::getSpeed(const char *type)
{
	return TelescopeBackend::getSpeed(type);
}

void Model::setSpeed(const char *type, double speed)
{
	TelescopeBackend::setSpeed(type, speed);
}

int Model::getConfigAll(ConfigItem* configs, int maxConfig)
{
	return TelescopeBackend::getConfigAll(configs, maxConfig);
}
