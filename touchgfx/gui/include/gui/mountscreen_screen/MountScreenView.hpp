#ifndef MOUNTSCREEN_VIEW_HPP
#define MOUNTSCREEN_VIEW_HPP

#include <gui_generated/mountscreen_screen/MountScreenViewBase.hpp>
#include <gui/mountscreen_screen/MountScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <touchgfx/widgets/ButtonWithIcon.hpp>
#include <gui/widgets/StarMapWidget.h>
#include <gui/containers/ConfigPopup.hpp>
#include <gui/containers/CoordinatePopup.hpp>

class MountScreenView: public MountScreenViewBase, public BaseScreenAdaptor
{
public:
	MountScreenView();
	virtual ~MountScreenView()
	{
	}
	virtual void setupScreen();
	virtual void tearDownScreen();
	virtual void updateDisplay(const EquatorialCoordinates &eq, const MountCoordinates& meq);
	void handleClickEvent(const ClickEvent& evt)
	{
		if (evt.getType() == ClickEvent::PRESSED)
		{
			lastPressed.x = evt.getX();
			lastPressed.y = evt.getY();
		}
		Screen::handleClickEvent(evt);
	}
	void handleGestureEvent(const GestureEvent& evt)
	{
		if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL && evt.getVelocity() > MIN_SWIPE_VELOCITY && !joyStick1.getRect().intersect(lastPressed.x, lastPressed.y)
				&& !sliderSpeed.getRect().intersect(lastPressed.x, lastPressed.y) && !cpop.isVisible()
				&& !coordpop.isVisible())
		{
			application().gotoHomeScreenScreenSlideTransitionWest();
		}
		Screen::handleGestureEvent(evt);
	}

	static const int NUM_SPEEDS = 5;
	static double SPEEDS[NUM_SPEEDS];
protected:

	StarMapWidget starmap;
	Ticker ticker;

	ConfigItem config_edit;bool update;

	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonStopCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> toggleTrackCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> toggleTrackSpeedCallback;
	touchgfx::Callback<MountScreenView, const TextAreaWithOneWildcard &, const ClickEvent &> callbackSlewSpeed;
	touchgfx::Callback<MountScreenView, ConfigItem *, bool> callbackSlewSpeedConfirmed;

	touchgfx::Callback<MountScreenView, const TextAreaWithTwoWildcards &, const ClickEvent &> callbackEqCoord;
	touchgfx::Callback<MountScreenView, CoordinatePopup::Coordinate, bool> callbackEqCoordGoto;

	touchgfx::Callback<MountScreenView, const TextAreaWithTwoWildcards &, const ClickEvent &> callbackMountCoord;
	touchgfx::Callback<MountScreenView, CoordinatePopup::Coordinate, bool> callbackMountCoordGoto;

	touchgfx::Callback<MountScreenView, const Slider &, int> callbackSliderSpeed;

	void buttonNWSEPressed(const AbstractButton& src);
	void buttonNWSEReleased(const AbstractButton& src);
	void buttonStopPressed(const AbstractButton& src);

	void trackToggled(const AbstractButton& src);

	void trackSpeedSelected(const AbstractButton& src);

	void setSlewSpeed(double speed);

	void slewSpeedClicked(const TextAreaWithOneWildcard &, const ClickEvent &evt);

	void slewSpeedSet(ConfigItem *, bool);

	void sliderSpeedChanged(const Slider &, int);

	void eqCoordClicked(const TextAreaWithTwoWildcards &, const ClickEvent &evt);

	void eqCoordGoto(CoordinatePopup::Coordinate c, bool ok);

	void mountCoordClicked(const TextAreaWithTwoWildcards &, const ClickEvent &evt);

	void mountCoordGoto(CoordinatePopup::Coordinate c, bool ok);

	ConfigPopup cpop;
	CoordinatePopup coordpop;

	struct
	{
		int x;
		int y;
	} lastPressed;

	volatile bool speedUpdateHardware;

};

#endif // MOUNTSCREEN_VIEW_HPP
