#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "TelescopeBackend.h"

Model::Model() :
		modelListener(0)
{
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
			modelListener->setTime(time(NULL));
			modelListener->setEqCoords(TelescopeBackend::getEqCoords());
			timeupdate = 10;
		}

		if (timesync-- == 0){
			TelescopeBackend::syncTime();
			timesync = 10000;
		}

	}
}

time_t Model::getTime()
{
	return time(NULL);
}
