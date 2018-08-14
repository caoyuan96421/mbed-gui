#include <gui/mountscreen_screen/MountScreenView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "BitmapDatabase.hpp"
#include "TelescopeBackend.h"

const double lunar_speed = 0.00402557046; // deg/s
const double solar_speed = 0.004166667; // deg/s

MountScreenView::MountScreenView() :
		update(false), buttonStopCallback(this, &MountScreenView::buttonStopPressed), toggleTrackCallback(this, &MountScreenView::trackToggled), toggleTrackSpeedCallback(this,
				&MountScreenView::trackSpeedSelected), callbackSlewSpeed(this, &MountScreenView::slewSpeedClicked), callbackSlewSpeedConfirmed(this, &MountScreenView::slewSpeedSet), callbackEqCoord(
				this, &MountScreenView::eqCoordClicked), callbackEqCoordGoto(this, &MountScreenView::eqCoordGoto), callbackMountCoord(this, &MountScreenView::mountCoordClicked), callbackMountCoordGoto(
				this, &MountScreenView::mountCoordGoto)
{
	baseview.addTo(&container);

	starmap.setPosition(15, 400, 150, 300);
	add(starmap);

	starmap.setFOV(2);
	starmap.setTouchable(false);
	starmap.setVisible(true); // Enable when transition finishes

	button_stop.setAction(buttonStopCallback);

	toggle_track.setAction(toggleTrackCallback);

	toggleSidereal.setAction(toggleTrackSpeedCallback);
	toggleLunar.setAction(toggleTrackSpeedCallback);
	toggleSolar.setAction(toggleTrackSpeedCallback);
	toggleKing.setAction(toggleTrackSpeedCallback);

	slewspeed.setClickAction(callbackSlewSpeed);
	eqcoords.setClickAction(callbackEqCoord);
	mount_coords.setClickAction(callbackMountCoord);

	cpop.setXY(0, 0);
	add(cpop);

	coordpop.setXY(0, 0);
	add(coordpop);

	joyStick1.setPositionChangedCallback(TelescopeBackend::handleNudge);
}

void MountScreenView::setupScreen()
{

	toggle_track.forceState((TelescopeBackend::getStatus() & TelescopeBackend::MOUNT_TRACKING) != 0);
	double trackSpeed = TelescopeBackend::getSpeed("track");
	if (presenter->isUseKingRate())
	{
		toggleKing.forceState(true);
	}
	else if (fabs(trackSpeed - 1.0) < 1e-3)
	{
		toggleSidereal.forceState(true);
	}
	else if (fabs(trackSpeed - lunar_speed / sidereal_speed) < 1e-3)
	{
		toggleLunar.forceState(true);
	}
	else if (fabs(trackSpeed - solar_speed / sidereal_speed) < 1e-3)
	{
		toggleSolar.forceState(true);
	}

	setSlewSpeed(TelescopeBackend::getSpeed("slew"));
	update = true;

//	ticker.attach(callback(_showStarMap, &starmap), 0.6f);
}

void MountScreenView::tearDownScreen()
{

}

void MountScreenView::updateDisplay(const EquatorialCoordinates& eq, const MountCoordinates& mc)
{
	if (update)
	{
		starmap.aimAt(eq.ra, eq.dec);

		char we = (eq.ra > 0) ? 'E' : 'W';
		char ns = (eq.dec > 0) ? 'N' : 'S';
		double r = (eq.ra < 0) ? eq.ra + 360.0 : eq.ra;
		double d = fabs(eq.dec);

		char buf[32];

		// RA string
		snprintf(buf, sizeof(buf), "%2dh%02d'%02d\"%c", int(r / 15), int(fmod(r, 15.0) * 4), (int) round(fmod(r, 0.25) * 240), we);
		Unicode::strncpy(eqcoordsBuffer1, buf, EQCOORDSBUFFER1_SIZE);

		// DEC string
		snprintf(buf, sizeof(buf), "%2d\x00b0%02d'%02d\"%c", int(d), int(fmod(d, 1.0) * 60), (int) round(fmod(d, 1.0 / 60) * 3600), ns);
		Unicode::strncpy(eqcoordsBuffer2, buf, EQCOORDSBUFFER2_SIZE);
		eqcoords.invalidate();

		we = (mc.ra_delta > 0) ? '+' : '-';
		ns = (mc.dec_delta > 0) ? '+' : '-';
		r = fabs(mc.ra_delta);
		d = fabs(mc.dec_delta);

		// RA string
		snprintf(buf, sizeof(buf), "%c%2d\x00b0%02d'%02d\"", we, int(r), int(fmod(r, 1.0) * 60), (int) round(fmod(r, 1.0 / 60) * 3600));
		Unicode::strncpy(mount_coordsBuffer1, buf, MOUNT_COORDSBUFFER1_SIZE);

		// DEC string
		snprintf(buf, sizeof(buf), "%c%2d\x00b0%02d'%02d\"", ns, int(d), int(fmod(d, 1.0) * 60), (int) round(fmod(d, 1.0 / 60) * 3600));
		Unicode::strncpy(mount_coordsBuffer2, buf, MOUNT_COORDSBUFFER2_SIZE);
		mount_coords.invalidate();

		// Update king rate if selected
		if (presenter->isUseKingRate())
		{
			double kingRate = CelestialMath::kingRate(eq, presenter->getLocation(), time(NULL));
			TelescopeBackend::setSpeed("track", kingRate / sidereal_speed);
		}
	}
}


void MountScreenView::buttonNWSEReleased(const AbstractButton& src)
{
	TelescopeBackend::stopNudge();
}

void MountScreenView::buttonStopPressed(const AbstractButton& src)
{
	TelescopeBackend::emergencyStop();
	toggle_track.forceState(false);
	toggle_track.invalidate();
}

void MountScreenView::trackToggled(const AbstractButton& src)
{
	ToggleButton& tb = (ToggleButton &) src;
	if (tb.getState())
	{
		// Start tracking
		presenter->track(true);
	}
	else
	{
		presenter->track(false);
	}
}

void MountScreenView::trackSpeedSelected(const AbstractButton& src)
{
	ToggleButton &tb = (ToggleButton &) src;
	// Set all to deselected
	toggleSidereal.forceState(false);
	toggleLunar.forceState(false);
	toggleSolar.forceState(false);
	toggleKing.forceState(false);
	// Set the selected to selected state
	tb.forceState(true);
	bool useKing = false;
	if (&tb == &toggleSidereal)
	{
		presenter->setSpeed("track", 1.0);
	}
	else if (&src == &toggleLunar)
	{
		presenter->setSpeed("track", lunar_speed / sidereal_speed);
	}
	else if (&src == &toggleSolar)
	{
		presenter->setSpeed("track", solar_speed / sidereal_speed);
	}
	else if (&src == &toggleKing)
	{
		useKing = true;
		double kingRate = CelestialMath::kingRate(presenter->getEqCoords(), presenter->getLocation(), time(NULL));
		presenter->setSpeed("track", kingRate / sidereal_speed);
	}
	presenter->useKingRate(useKing);

	toggleSidereal.invalidate();
	toggleLunar.invalidate();
	toggleSolar.invalidate();
	toggleKing.invalidate();
}

void MountScreenView::setSlewSpeed(double speed)
{
	Unicode::UnicodeChar buf[16];
	if (speed > 64 * sidereal_speed)
	{
		Unicode::snprintfFloat(buf, sizeof(buf), "%.3f", speed);
		Unicode::snprintf(slewspeedBuffer, SLEWSPEED_SIZE, "%s \xB0/s", buf);
		slewspeed_star.setVisible(false);
	}
	else
	{
		Unicode::snprintfFloat(buf, sizeof(buf), "%.1f", speed / sidereal_speed);
		Unicode::snprintf(slewspeedBuffer, SLEWSPEED_SIZE, "x%s      ", buf);
		slewspeed_star.setVisible(true);
	}
	slewspeed.invalidate();
}

void MountScreenView::slewSpeedClicked(const TextAreaWithOneWildcard&, const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::RELEASED && cpop.isVisible() == false)
	{
		config_edit.setConfig("slew_speed");
		config_edit.setName("Slew Speed (deg/s)");
		config_edit.setHelp("Slew Speed in degrees per second. ");
		config_edit.type = DATATYPE_DOUBLE;
		config_edit.value.ddata = presenter->getSpeed("slew");
		config_edit.min.ddata = 0;
		config_edit.max.ddata = presenter->getConfigDouble("max_speed");

		starmap.setVisible(false);
		cpop.editConfig(&config_edit);
		cpop.setCallback(&callbackSlewSpeedConfirmed);
	}
}

void MountScreenView::slewSpeedSet(ConfigItem* config, bool ok)
{
	if (ok)
	{
		double speed = config->value.ddata;
		presenter->setSpeed("slew", speed);
		setSlewSpeed(presenter->getSpeed("slew"));
	}
	starmap.invalidate();
	starmap.setVisible(true);
	starmap.invalidate();
}

void MountScreenView::eqCoordClicked(const TextAreaWithTwoWildcards&, const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::RELEASED && coordpop.isVisible() == false)
	{
		coordpop.setCallback(&callbackEqCoordGoto);
		coordpop.show(CoordinatePopup::FORMAT_H_M_S_UNSIGNED, CoordinatePopup::FORMAT_D_M_S_NS, "Right Ascension", "Declination");
		starmap.setVisible(false);
	}
}

void MountScreenView::eqCoordGoto(CoordinatePopup::Coordinate c, bool ok)
{
	if (ok)
	{
		printf("Goto ");
		EquatorialCoordinates(c.v2, c.v1).print(stdout);
		printf("\r\n");

		TelescopeBackend::goTo(EquatorialCoordinates(c.v2, c.v1));
	}
	starmap.invalidate();
	starmap.setVisible(true);
	starmap.invalidate();
}

void MountScreenView::mountCoordClicked(const TextAreaWithTwoWildcards&, const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::RELEASED && coordpop.isVisible() == false)
	{
		coordpop.setCallback(&callbackMountCoordGoto);
		coordpop.show(CoordinatePopup::FORMAT_D_M_S_SIGNED, CoordinatePopup::FORMAT_D_M_S_SIGNED, "RA Axis", "DEC Axis");
		starmap.setVisible(false);
	}
}

void MountScreenView::mountCoordGoto(CoordinatePopup::Coordinate c, bool ok)
{
	if (ok)
	{
		printf("Goto mount ra=%f, dec=%f\r\n", c.v1, c.v2);
		TelescopeBackend::goToMount(MountCoordinates(c.v2, c.v1));
	}
	starmap.invalidate();
	starmap.setVisible(true);
	starmap.invalidate();
}
