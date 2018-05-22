#ifndef MOUNTSCREEN_VIEW_HPP
#define MOUNTSCREEN_VIEW_HPP

#include <gui_generated/mountscreen_screen/MountScreenViewBase.hpp>
#include <gui/mountscreen_screen/MountScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <gui/widgets/StarMapWidget.h>

class MountScreenView: public MountScreenViewBase, public BaseScreenAdaptor
{
public:
	MountScreenView();
	virtual ~MountScreenView()
	{
	}
	virtual void setupScreen();
	virtual void tearDownScreen();
	virtual void setCoords(const EquatorialCoordinates &eq, const MountCoordinates& meq);
protected:

	StarMapWidget starmap;

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

	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonNWSEPressedCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonNWSEReleasedCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> buttonStopCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> toggleTrackCallback;
	touchgfx::Callback<MountScreenView, const AbstractButton&> toggleTrackSpeedCallback;

	void buttonNWSEPressed(const AbstractButton& src);
	void buttonNWSEReleased(const AbstractButton& src);
	void buttonStopPressed(const AbstractButton& src);

	void trackToggled(const AbstractButton& src);

	void trackSpeedSelected(const AbstractButton& src);

	void setSlewSpeed(double speed);
};

#endif // MOUNTSCREEN_VIEW_HPP
