#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() :
		modelListener(0)
{
}

void Model::tick()
{
	static int timeupdate = 0;
	if (modelListener)
	{
		if (!(timeupdate--))
		{
			// Update time every 10 ticks
			modelListener->setTime(time(NULL));
			timeupdate = 10;
		}

	}
}

time_t Model::getTime()
{
	return time(NULL);
}
