#ifndef ALIGNSCREEN_VIEW_HPP
#define ALIGNSCREEN_VIEW_HPP

#include <gui_generated/alignscreen_screen/AlignScreenViewBase.hpp>
#include <gui/alignscreen_screen/AlignScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <touchgfx/Callback.hpp>
#include <gui/widgets/ButtonItem.h>
#include "StarCatalog.h"
#include "CelestialMath.h"

class AlignScreenView: public AlignScreenViewBase, public BaseScreenAdaptor
{
public:
	AlignScreenView();
	virtual ~AlignScreenView()
	{
	}
	virtual void setupScreen();
	virtual void tearDownScreen();
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
		if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL && evt.getVelocity() > MIN_SWIPE_VELOCITY && !scrollableContainer1.getRect().intersect(lastPressed.x, lastPressed.y)
				&& !scrollableContainer2.getRect().intersect(lastPressed.x, lastPressed.y) && !joyStick2.getRect().intersect(lastPressed.x, lastPressed.y)
				&& !sliderSpeed.getRect().intersect(lastPressed.x, lastPressed.y))
		{
			application().gotoUtilityScreenScreenSlideTransitionWest();
		}
		Screen::handleGestureEvent(evt);
	}

protected:

	void updateMenu();
	void updateCalibration();
	void buttonRefreshPressed(const AbstractButton&)
	{
		updateMenu();
	}
	void starSelected(const AbstractButton&);
	void buttonAddPressed(const AbstractButton&);
	void buttonDeletePressed(const AbstractButton&);
	void buttonGotoPressed(const AbstractButton&);
	void buttonAlignPressed(const AbstractButton&);
	void buttonStopPressed(const AbstractButton&);
	void sliderSpeedChanged(const Slider &, int);

	static void callback(StarInfo*, void*);
	void _callback(StarInfo *);

	void clearMenu(Container &);

	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonRefreshCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonAddCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonDeleteCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonGotoCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonAlignCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonStopCallback;

	touchgfx::Callback<AlignScreenView, const AbstractButton&> starSelectedCallback;

	touchgfx::Callback<AlignScreenView, const Slider &, int> callbackSliderSpeed;

	touchgfx::Container container_candidates;
	touchgfx::Container container_selected;

	time_t time_now;
	LocationCoordinates loc;

	float minAlt;
	int height;
	int num_candidates;
	int num_alignment;
	ButtonItem *selectedCandidate;
	ButtonItem *selectedAlignment;

	colortype defaultColor, selectedColor, pressedColor;

	struct
	{
		int x;
		int y;
	} lastPressed;
};

#endif // ALIGNSCREEN_VIEW_HPP
