#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include <gui_generated/containers/JoyStickBase.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/widgets/AbstractButton.hpp>

class JoyStick: public JoyStickBase
{
public:
	JoyStick();
	virtual ~JoyStick()
	{
	}

	void handleClickEvent(const ClickEvent &);
	void handleDragEvent(const DragEvent &);

	void resetStick();

	void setPositionChangedCallback(void (*cb)(float, float))
	{
		posChangedCallback = cb;
	}

	static const float thd = 0.667; // Threshold

protected:

	// Current speed
	int xcurr;
	int ycurr;
	int zone;

	void (*posChangedCallback)(float, float);

	void updateStickPosition(int x, int y);
};

#endif // JOYSTICK_HPP
