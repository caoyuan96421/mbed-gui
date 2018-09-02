#ifndef POLARALIGNSCREEN_VIEW_HPP
#define POLARALIGNSCREEN_VIEW_HPP

#include <gui_generated/polaralignscreen_screen/PolarAlignScreenViewBase.hpp>
#include <gui/polaralignscreen_screen/PolarAlignScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <gui/widgets/ButtonItem.h>
#include "StarCatalog.h"
#include "CelestialMath.h"

class PolarAlignScreenView: public PolarAlignScreenViewBase, public BaseScreenAdaptor
{
public:
	PolarAlignScreenView();
	virtual ~PolarAlignScreenView();
	virtual void setupScreen();
	virtual void tearDownScreen();
	void handleGestureEvent(const GestureEvent& evt)
	{
		if (evt.getType() == GestureEvent::SWIPE_HORIZONTAL && evt.getVelocity() > MIN_SWIPE_VELOCITY)
		{
			application().gotoUtilityScreenScreenSlideTransitionWest();
		}
	}
protected:

	enum
	{
		RESET = 0, STEP1_CHOOSESTAR, STEP2_SLEWTOTARGET, STEP3_COMPLETEPA, ERROR
	} pa_state;

	void listStars();
	void slewToSelected();
	void completePA();

	touchgfx::Callback<PolarAlignScreenView, const AbstractButton&> buttonCallback;
	void buttonPressed(const AbstractButton &);

	touchgfx::Callback<PolarAlignScreenView, const AbstractButton&> starSelectedCallback;
	void starSelected(const AbstractButton&);

	touchgfx::Container container_stars;

	time_t time_now;
	LocationCoordinates loc;
	float minAlt;
	int height;
	int num_stars;
	ButtonItem *selectedStar;
	colortype defaultColor, selectedColor, pressedColor;
	EqCalibration *calib;

	static void callback(StarInfo* s, void* arg);
	void _callback(StarInfo* star);
};

#endif // POLARALIGNSCREEN_VIEW_HPP
