#include <gui/containers/JoyStick.hpp>

JoyStick::JoyStick()
{
	this->setTouchable(true);
	posChangedCallback = NULL;
	xcurr = 0;
	ycurr = 0;
	zone = 0;
}

void JoyStick::handleClickEvent(const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::PRESSED)
	{
		updateStickPosition(evt.getX(), evt.getY());
	}
	else
	{
		resetStick();
	}
}

void JoyStick::handleDragEvent(const DragEvent& evt)
{
	if (evt.getType() == DragEvent::DRAGGED)
	{
		int x = evt.getNewX();
		int y = evt.getNewY();
		if (x < 0)
			x = 0;
		if (x > getWidth())
			x = getWidth();
		if (y < 0)
			y = 0;
		if (y > getHeight())
			y = getHeight();
		updateStickPosition(x, y);
	}
}

void JoyStick::resetStick()
{
	updateStickPosition(getWidth() / 2, getHeight() / 2);
}

static inline float sgn(float x)
{
	if (x > 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

static float f(float x)
{
	return powf(fabsf(x) / JoyStick::thd, 3) * sgn(x);
}

void JoyStick::updateStickPosition(int x, int y)
{
	handle.setX(x - handle.getWidth() / 2);
	handle.setY(y - handle.getHeight() / 2);
	invalidate();

	if (x == getWidth() / 2 && y == getHeight() / 2)
	{
		// Reset position
		if (posChangedCallback)
		{
			posChangedCallback(0, 0);
		}
		xcurr = 0;
		ycurr = 0;
		zone = 0;
	}
	else
	{
		// Calculate zone
		float dX = ((float) x / getWidth() - 0.5f) * 2;
		float dY = -((float) y / getHeight() - 0.5f) * 2;
		float xval = 0, yval = 0;
		if (dX < -1)
			dX = -1;
		if (dX > 1)
			dX = 1;
		if (dY < -1)
			dY = -1;
		if (dY > 1)
			dY = 1;

		if (fabsf(dX) <= thd && fabsf(dY) <= thd)
		{
			// Zone 1-4
			if (dY <= dX && dY > -dX)
			{
				// Zone 1
				zone = 1;
				xval = f(dX);
			}
			else if (dY > dX && dY > -dX)
			{
				// Zone 2
				zone = 2;
				yval = f(dY);
			}
			else if (dY > dX && dY <= -dX)
			{
				// Zone 3
				zone = 3;
				xval = f(dX);
			}
			else
			{
				// Zone 4
				zone = 4;
				yval = f(dY);
			}
		}
		else if (fabsf(dX) >= thd && fabsf(dY) >= thd)
		{
			// Zone 9-12
			if (dX > 0 && dY > 0)
			{
				// Zone 9
				zone = 9;
				xval = 1;
				yval = 1;
			}
			else if (dX < 0 && dY > 0)
			{
				// Zone 10
				zone = 10;
				xval = -1;
				yval = 1;
			}
			else if (dX < 0 && dY < 0)
			{
				// Zone 11
				zone = 11;
				xval = -1;
				yval = -1;
			}
			else
			{
				// Zone 12
				zone = 12;
				xval = 1;
				yval = -1;
			}
		}
		else
		{
			// Zone 5-8
			if (dX > thd)
			{
				// Zone 5
				zone = 5;
				xval = 1;
			}
			else if (dY > thd)
			{
				// Zone 6
				zone = 6;
				yval = 1;
			}
			else if (dX < -thd)
			{
				// Zone 7
				zone = 7;
				xval = -1;
			}
			else if (dY < -thd)
			{
				// Zone 8
				zone = 8;
				yval = -1;
			}
		}

		if ((xcurr != xval || ycurr != yval) && posChangedCallback)
		{
			posChangedCallback(xval, yval);
		}

		xcurr = xval;
		ycurr = yval;
	}
}
