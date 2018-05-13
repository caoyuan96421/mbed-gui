/*
 * ConfigButton.cpp
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#include <gui/widgets/ConfigButton.h>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/hal/Types.hpp>
#include <cstdio>
using namespace touchgfx;

ConfigButton::ConfigButton(ConfigItem &config,
		ConfigPopup &popup) :
		config(config), callback(this, &ConfigButton::buttonPressed), popup(
				popup)
{
	setLabelText(TypedText(T_MEDIUM));
	Unicode::strncpy(config_name, config.name, CONFIG_NAME_SIZE);
	setLabelColor(Color::getColorFrom24BitRGB(255, 48, 48), false);
	setLabelColorPressed(Color::getColorFrom24BitRGB(255, 255, 255), false);

	this->setAction(callback);
}

void ConfigButton::buttonPressed(const AbstractButton &source)
{
	popup.editConfig(&config);
	printf("Button pressed: %s\n", config.name);
}

void ConfigButton::draw(const Rect& area) const
{
//	Button::draw(area);
	int margin = 5;
	if (getWidth() <= margin * 2 || getHeight() <= margin * 2)
		margin = 0;
	Rect bgrect(margin, 0, getWidth() - margin * 2, getHeight() - margin * 2);
	bgrect &= area;
	if (!bgrect.isEmpty())
	{
		translateRectToAbsolute(bgrect);
		HAL::lcd().fillRect(bgrect,
				!pressed ?
						Color::getColorFrom24BitRGB(40, 20, 20) :
						Color::getColorFrom24BitRGB(100, 30, 60));
	}

	const Font* fontToDraw = typedText.getFont();
	if ((fontToDraw != 0) && typedText.hasValidId())
	{
		uint8_t height = textHeightIncludingSpacing;
		int16_t offset;
		Rect labelRect;
		switch (rotation)
		{
		default:
		case TEXT_ROTATE_0:
		case TEXT_ROTATE_180:
			offset = (this->getHeight() - height) / 2;
			labelRect = Rect(margin * 2, offset, this->getWidth(), height);
			break;
		case TEXT_ROTATE_90:
		case TEXT_ROTATE_270:
			offset = (this->getWidth() - height) / 2;
			labelRect = Rect(offset, margin * 2, height, this->getHeight());
			break;
		}
		Rect dirty = labelRect & area;

		if (!dirty.isEmpty())
		{
			dirty.x -= labelRect.x;
			dirty.y -= labelRect.y;
			translateRectToAbsolute(labelRect);
			LCD::StringVisuals visuals(fontToDraw,
					pressed ? colorPressed : color, alpha,
					typedText.getAlignment(), 0, rotation,
					typedText.getTextDirection(), 0, WIDE_TEXT_NONE);
			HAL::lcd().drawString(labelRect, dirty, visuals,
					config_name);
		}
	}
}
