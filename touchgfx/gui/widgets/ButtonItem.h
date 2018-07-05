/*
 * ButtonItem.h
 *
 *  Created on: 2018Äê6ÔÂ29ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_WIDGETS_BUTTONITEM_H_
#define TOUCHGFX_GUI_WIDGETS_BUTTONITEM_H_

#include <touchgfx/widgets/AbstractButton.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/containers/Container.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/EasingEquations.hpp>
#include <touchgfx/TypedText.hpp>

namespace touchgfx
{


class ButtonItem: public AbstractButton
{
public:
	ButtonItem();
	virtual ~ButtonItem();

	void setName(Unicode::UnicodeChar *name);
	Unicode::UnicodeChar * getName();
	void setValue(Unicode::UnicodeChar *value);
	Unicode::UnicodeChar * getValue();

	void draw(const Rect &invalidated) const;
	Rect getSolidRect() const;

	void* getUserData() const
	{
		return userData;
	}

	void setUserData(void* userData)
	{
		this->userData = userData;
	}

	colortype getPressedColor() const
	{
		return pressedColor;
	}

	void setPressedColor(colortype pressedColor)
	{
		this->pressedColor = pressedColor;
	}

	colortype getReleasedColor() const
	{
		return releasedColor;
	}

	void setReleasedColor(colortype releasedColor)
	{
		this->releasedColor = releasedColor;
	}

protected:
	Unicode::UnicodeChar *name;
	Unicode::UnicodeChar *value;
	void *userData;
	colortype pressedColor;
	colortype releasedColor;
};

} /* namespace touchgfx */

#endif /* TOUCHGFX_GUI_WIDGETS_BUTTONITEM_H_ */
