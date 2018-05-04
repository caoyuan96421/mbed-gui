/*
 * ConfigButton.h
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_WIDGETS_CONFIGBUTTON_H_
#define TOUCHGFX_GUI_WIDGETS_CONFIGBUTTON_H_

#include <touchgfx/widgets/ButtonWithLabel.hpp>
#include <gui/include/gui/containers/ConfigPopup.hpp>
#include "TelescopeBackend.h"

class ConfigButton: public touchgfx::ButtonWithLabel
{
public:
	ConfigButton(ConfigItem &config, ConfigPopup &popup);
	virtual ~ConfigButton()
	{
	}

	void draw(const touchgfx::Rect& area) const;

protected:
	ConfigItem config;
	touchgfx::Unicode::UnicodeChar config_name[32];
	touchgfx::Callback<ConfigButton, const AbstractButton&> callback;
	ConfigPopup &popup;

	void buttonPressed(const AbstractButton &);
};

#endif /* TOUCHGFX_GUI_WIDGETS_CONFIGBUTTON_H_ */
