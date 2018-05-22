#ifndef MODEL_HPP
#define MODEL_HPP

#include <ctime>
#include "CelestialMath.h"
#include "PlanetMoon.h"
#include "TelescopeBackend.h"

class ModelListener;

/**
 * The Model class defines the data model in the model-view-presenter paradigm.
 * The Model is a singular object used across all presenters. The currently active
 * presenter will have a pointer to the Model through deriving from ModelListener.
 *
 * The Model will typically contain UI state information that must be kept alive
 * through screen transitions. It also usually provides the interface to the rest
 * of the system (the backend). As such, the Model can receive events and data from
 * the backend and inform the current presenter of such events through the modelListener
 * pointer, which is automatically configured to point to the current presenter.
 * Conversely, the current presenter can trigger events in the backend through the Model.
 */
class Model
{
public:
	Model();

	/**
	 * Sets the modelListener to point to the currently active presenter. Called automatically
	 * when switching screen.
	 */
	void bind(ModelListener* listener)
	{
		modelListener = listener;
	}

	/**
	 * This function will be called automatically every frame. Can be used to e.g. sample hardware
	 * peripherals or read events from the surrounding system and inject events to the GUI through
	 * the ModelListener interface.
	 */
	void tick();

	time_t getTime();

	EquatorialCoordinates getEqCoords();
	MountCoordinates getMountCoords();
	LocationCoordinates getLocation();
	int getTimeZone();
	TelescopeBackend::mountstatus_t getStatus();
	void track(bool);
	void syncNow()
	{
		syncImm = true;
	}

	int getConfigString(const char *config, char *buf, int size);
	int getConfigInt(const char *config);
	double getConfigDouble(const char *config);bool getConfigBool(const char *config);

	void setSpeed(const char *type, double speedSidereal);
	double getSpeed(const char *type);

	int getConfigAll(ConfigItem *configs, int maxConfig);

	void useKingRate(bool use)
	{
		king = use;
	}
	bool isUseKingRate()
	{
		return king;
	}

	// Get mount information
//    double getRA();
//    double getDEC();
//    double getSpeed();
//
//    int goTo(double ra, double dec);

protected:
	/**
	 * Pointer to the currently active presenter.
	 */
	volatile bool syncImm;bool king;
	EquatorialCoordinates eq_coord;
	MountCoordinates mount_coord;
	LocationCoordinates location;
	int timezone;
	ModelListener* modelListener;
};

#endif /* MODEL_HPP */

