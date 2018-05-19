#include <gui/mountscreen_screen/MountScreenView.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include "BitmapDatabase.hpp"
#include "TelescopeBackend.h"

MountScreenView::MountScreenView() :
		buttonNWSEPressedCallback(this, &MountScreenView::buttonNWSEPressed), buttonNWSEReleasedCallback(this, &MountScreenView::buttonNWSEReleased), buttonStopCallback(this,
				&MountScreenView::buttonStopPressed)
{
	baseview.addTo(&container);

	starmap.setPosition(starmap_bb.getX() + 1, starmap_bb.getY() + 1, starmap_bb.getWidth() - 2, starmap_bb.getHeight() - 2);
	add(starmap);

	starmap.setFOV(2);
//	starmap.setTouchable(false);

	button_north.setPosition(232, 412, 191, 87);
	button_north.setBitmaps(Bitmap(BITMAP_BUTTONUP_ID), Bitmap(BITMAP_BUTTONUP_PRESSED_ID), Bitmap(BITMAP_UP_ARROW_48_ID), Bitmap(BITMAP_UP_ARROW_48_ID));
	button_north.setIconXY(71, 29);

	button_south.setPosition(233, 600, 191, 87);
	button_south.setBitmaps(Bitmap(BITMAP_BUTTONDOWN_ID), Bitmap(BITMAP_BUTTONDOWN_PRESSED_ID), Bitmap(BITMAP_DOWN_ARROW_48_ID), Bitmap(BITMAP_DOWN_ARROW_48_ID));
	button_south.setIconXY(71, 29);

	button_east.setPosition(378, 454, 87, 191);
	button_east.setBitmaps(Bitmap(BITMAP_BUTTONRIGHT_ID), Bitmap(BITMAP_BUTTONRIGHT_PRESSED_ID), Bitmap(BITMAP_NEXT_ARROW_48_ID), Bitmap(BITMAP_NEXT_ARROW_48_ID));
	button_east.setIconXY(29, 71);

	button_west.setPosition(191, 455, 87, 191);
	button_west.setBitmaps(Bitmap(BITMAP_BUTTONLEFT_ID), Bitmap(BITMAP_BUTTONLEFT_PRESSED_ID), Bitmap(BITMAP_BACK_ARROW_48_ID), Bitmap(BITMAP_BACK_ARROW_48_ID));
	button_west.setIconXY(29, 71);

	button_north.setAction(buttonNWSEPressedCallback);
	button_east.setAction(buttonNWSEPressedCallback);
	button_south.setAction(buttonNWSEPressedCallback);
	button_west.setAction(buttonNWSEPressedCallback);

	button_north.setReleaseAction(buttonNWSEReleasedCallback);
	button_east.setReleaseAction(buttonNWSEReleasedCallback);
	button_south.setReleaseAction(buttonNWSEReleasedCallback);
	button_west.setReleaseAction(buttonNWSEReleasedCallback);

	button_stop.setAction(buttonStopCallback);

	add(button_north);
	add(button_south);
	add(button_east);
	add(button_west);
}

void MountScreenView::setupScreen()
{

}

void MountScreenView::tearDownScreen()
{

}

void MountScreenView::setCoords(const EquatorialCoordinates& eq, const MountCoordinates& mc)
{
//	BaseScreenAdaptor::setEqCoords(eq);
	starmap.aimAt(eq.ra, eq.dec);

	char we = (eq.ra > 0) ? 'E' : 'W';
	char ns = (eq.dec > 0) ? 'N' : 'S';
	double r = (eq.ra < 0) ? eq.ra + 360.0 : eq.ra;
	double d = fabs(eq.dec);

	char buf[32];

	// RA string
	snprintf(buf, sizeof(buf), "%02dh%02d'%02d\"%c", int(r / 15), int(fmod(r, 15.0) * 4), (int) round(fmod(r, 0.25) * 240), we);
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
	snprintf(buf, sizeof(buf), "%c%02d\x00b0%02d'%02d\"", we, int(r), int(fmod(r, 1.0) * 60), (int) round(fmod(r, 1.0 / 60) * 3600));
	Unicode::strncpy(mount_coordsBuffer1, buf, MOUNT_COORDSBUFFER1_SIZE);

	// DEC string
	snprintf(buf, sizeof(buf), "%c%2d\x00b0%02d'%02d\"", ns, int(d), int(fmod(d, 1.0) * 60), (int) round(fmod(d, 1.0 / 60) * 3600));
	Unicode::strncpy(mount_coordsBuffer2, buf, MOUNT_COORDSBUFFER2_SIZE);
	mount_coords.invalidate();
}

void MountScreenView::buttonNWSEPressed(const AbstractButton& src)
{
	TelescopeBackend::Direction dir;
	if (&src == &button_east)
	{
		dir = TelescopeBackend::EAST;
	}
	else if (&src == &button_west)
	{
		dir = TelescopeBackend::WEST;
	}
	else if (&src == &button_north)
	{
		dir = TelescopeBackend::NORTH;
	}
	else if (&src == &button_south)
	{
		dir = TelescopeBackend::SOUTH;
	}
	TelescopeBackend::startNudge(dir);
}

void MountScreenView::buttonNWSEReleased(const AbstractButton& src)
{
	TelescopeBackend::stopNudge();
}

void MountScreenView::buttonStopPressed(const AbstractButton& src)
{
	TelescopeBackend::emergencyStop();
}
