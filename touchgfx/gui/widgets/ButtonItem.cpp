/*
 * ButtonItem.cpp
 *
 *  Created on: 2018Äê6ÔÂ29ÈÕ
 *      Author: caoyuan9642
 */

#include <ButtonItem.h>
#include <Color.hpp>
#include <cstring>
#include <cstdio>
#include "texts/TextKeysAndLanguages.hpp"
using namespace touchgfx;

ButtonItem::ButtonItem() :
		name(NULL), value(NULL), userData(NULL)
{
	releasedColor = Color::getColorFrom24BitRGB(10, 10, 10);
	pressedColor = Color::getColorFrom24BitRGB(50, 50, 50);
}

ButtonItem::~ButtonItem()
{
}

void ButtonItem::setName(Unicode::UnicodeChar* name)
{
	this->name = name;
}

Unicode::UnicodeChar* ButtonItem::getName()
{
	return name;
}

void ButtonItem::setValue(Unicode::UnicodeChar* value)
{
	this->value = value;
}

Unicode::UnicodeChar* ButtonItem::getValue()
{
	return value;
}

void ButtonItem::draw(const Rect& invalidated) const
{
	Rect dirty(0, 0, getWidth(), getHeight());
	dirty &= invalidated;
	colortype textcolor = Color::getColorFrom24BitRGB(128, 10, 10);
	uint8_t alpha = 255;
	if (!dirty.isEmpty())
	{
		translateRectToAbsolute(dirty);
		HAL::lcd().fillRect(dirty, pressed ? pressedColor : releasedColor, alpha);
	}

	TypedText typedText(T_SMALL);
	const Font* fontToDraw = typedText.getFont();
	uint8_t w = fontToDraw->getCharWidth(' ');
	if (name)
	{
		uint8_t height = fontToDraw->getMaxTextHeight(name);
		int16_t offset;
		Rect labelRect;
		offset = (getHeight() - height) / 2;
		labelRect = Rect(4 * w, offset, getWidth() - 5 * w, height);

		dirty = labelRect & invalidated;

		if (!dirty.isEmpty())
		{
			dirty.x -= labelRect.x;
			dirty.y -= labelRect.y;
			translateRectToAbsolute(labelRect);
			LCD::StringVisuals visuals(fontToDraw, textcolor, alpha, LEFT, 0, TEXT_ROTATE_0, typedText.getTextDirection(), 0, WIDE_TEXT_NONE);
			HAL::lcd().drawString(labelRect, dirty, visuals, name);
		}
	}

	if (value)
	{
		uint8_t height = fontToDraw->getMaxTextHeight(value);
		int16_t offset = (getHeight() - height) / 2 - 2;
		Rect labelRect = Rect(4 * w, offset, getWidth() - 5 * w, height);

		dirty = labelRect & invalidated;

		if (!dirty.isEmpty())
		{
			dirty.x -= labelRect.x;
			dirty.y -= labelRect.y;
			translateRectToAbsolute(labelRect);
			LCD::StringVisuals visuals(fontToDraw, textcolor, alpha, LEFT, 0, TEXT_ROTATE_0, typedText.getTextDirection(), 0, WIDE_TEXT_NONE);
			HAL::lcd().drawString(labelRect, dirty, visuals, value);
		}
	}
}
