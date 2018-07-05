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
protected:

	StarMapWidget starmap;
	Ticker ticker;

	class ButtonWithIconEx: public ButtonWithIcon
	{
	public:
		void setReleaseAction(GenericCallback<const AbstractButton&>& callback)
		{
			release_action = &callback;
		}
		virtual void handleClickEvent(const ClickEvent& event)
		{
			bool wasPressed = pressed;
			pressed = (event.getType() == ClickEvent::PRESSED);
			if ((pressed && !wasPressed) || (!pressed && wasPressed))
			{
				// Pressed state changed, so invalidate
				invalidate();
			}
			if (!wasPressed && pressed && action)
			{
				// This is a click. Fire callback.
				if (action->isValid())
				{
					action->execute(*this);
				}
			}
			else if (wasPressed && !pressed && release_action)
			{
				if (release_action->isValid())
				{
					release_action->execute(*this);
				}
			}
		}
	protected:
		GenericCallback<const AbstractButton&>* release_action;
	} button_north, button_south, button_east, button_west;

	ConfigItem config_edit;bool update;

	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonNWSEPressedCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonNWSEReleasedCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonStopCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> toggleTrackCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> toggleTrackSpeedCallback;
	touchgfx::Callback<MountScreenView, const TextAreaWithOneWildcard &, const ClickEvent &> callbackSlewSpeed;
	touchgfx::Callback<MountScreenView, ConfigItem *, bool> callbackSlewSpeedConfirmed;

	touchgfx::Callback<MountScreenView, const TextAreaWithTwoWildcards &, const ClickEvent &> callbackEqCoord;
	touchgfx::Callback<MountScreenView, CoordinatePopup::Coordinate, bool> callbackEqCoordGoto;

	touchgfx::Callback<MountScreenView, const TextAreaWithTwoWildcards &, const ClickEvent &> callbackMountCoord;
	touchgfx::Callback<MountScreenView, CoordinatePopup::Coordinate, bool> callbackMountCoordGoto;

	void buttonNWSEPressed(const AbstractButton& src);
	void buttonNWSEReleased(const AbstractButton& src);
	void buttonStopPressed(const AbstractButton& src);

	void trackToggled(const AbstractButton& src);

	void trackSpeedSelected(const AbstractButton& src);

	void setSlewSpeed(double speed);

	void slewSpeedClicked(const TextAreaWithOneWildcard &, const ClickEvent &evt);

	void slewSpeedSet(ConfigItem *, bool);

	void eqCoordClicked(const TextAreaWithTwoWildcards &, const ClickEvent &evt);

	void eqCoordGoto(CoordinatePopup::Coordinate c, bool ok);

	void mountCoordClicked(const TextAreaWithTwoWildcards &, const ClickEvent &evt);

	void mountCoordGoto(CoordinatePopup::Coordinate c, bool ok);

	ConfigPopup cpop;
	CoordinatePopup coordpop;
};

#endif // MOUNTSCREEN_VIEW_HPP
