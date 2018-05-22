#ifndef BASESCREEN_PRESENTER_HPP
#define BASESCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class BaseScreenView;

class BaseScreenPresenter: public Presenter, public ModelListener
{
public:
	BaseScreenPresenter(BaseScreenView& v);

	/**
	 * The activate function is called automatically when this screen is "switched in"
	 * (ie. made active). Initialization logic can be placed here.
	 */
	virtual void activate();

	/**
	 * The deactivate function is called automatically when this screen is "switched out"
	 * (ie. made inactive). Teardown functionality can be placed here.
	 */
	virtual void deactivate();

	virtual ~BaseScreenPresenter()
	{
	}
	;

	EquatorialCoordinates getEqCoords()
	{
		return model->getEqCoords();
	}

	MountCoordinates getMountCoords()
	{
		return model->getMountCoords();
	}

	LocationCoordinates getLocation()
	{
		return model->getLocation();
	}

	TelescopeBackend::mountstatus_t getStatus()
	{
		return model->getStatus();
	}

	void track(bool on)
	{
		return model->track(on);
	}

	void setSpeed(const char *type, double speed)
	{
		model->setSpeed(type, speed);
	}

	int getConfigString(const char *config, char *buf, int size)
	{
		return model->getConfigString(config, buf, size);
	}
	int getConfigInt(const char *config)
	{
		return model->getConfigInt(config);
	}
	double getConfigDouble(const char *config)
	{
		return model->getConfigDouble(config);
	}
	bool getConfigBool(const char *config)
	{
		return model->getConfigBool(config);
	}

	double getSpeed(const char *type)
	{
		return model->getSpeed(type);
	}

	void useKingRate(bool use)
	{
		model->useKingRate(use);
	}

	bool isUseKingRate()
	{
		return model->isUseKingRate();
	}

	int getConfigAll(ConfigItem *configs, int maxConfig)
	{
		return model->getConfigAll(configs, maxConfig);
	}

	void setTime(time_t timestamp, int tz);
	void setCoords(const EquatorialCoordinates &eq, const MountCoordinates& meq);

protected:
	BaseScreenView& view;

private:
	BaseScreenPresenter();

};

#endif // BASESCREEN_PRESENTER_HPP
