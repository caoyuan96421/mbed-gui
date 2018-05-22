#ifndef SETTINGSCREEN_VIEW_HPP
#define SETTINGSCREEN_VIEW_HPP

#include <gui_generated/settingscreen_screen/SettingScreenViewBase.hpp>
#include <gui/settingscreen_screen/SettingScreenPresenter.hpp>
#include <gui/basescreen_screen/BaseScreenView.hpp>
#include <gui/widgets/ConfigButton.h>
#include <gui/widgets/AccordionMenu.h>
#include <gui/BaseScreenAdaptor.h>
#include "TelescopeBackend.h"

class SettingScreenView: public SettingScreenViewBase, public BaseScreenAdaptor
{
public:
	SettingScreenView();
	virtual ~SettingScreenView();
	virtual void setupScreen();
	virtual void tearDownScreen();
protected:
	static const int MAX_CONFIG = 64;
	ConfigItem configs[MAX_CONFIG];
	int num_config;

	Accordion accordion;
	AccordionMenu *hc_menu;
	AccordionMenu *mount_menu;
	AccordionItem *abuttons[MAX_CONFIG];


	Unicode::UnicodeChar name1[20];
	Unicode::UnicodeChar name2[20];
	Unicode::UnicodeChar name3[20];
	Unicode::UnicodeChar name4[20];

	touchgfx::Callback<SettingScreenView, const AccordionItem &> configCallback;

	void configButtonPressed(const AccordionItem &);
};

#endif // SETTINGSCREEN_VIEW_HPP
