#include <gui/settingscreen_screen/SettingScreenView.hpp>
#include "BitmapDatabase.hpp"
#include <cstdlib>
#include <cstring>

using namespace touchgfx;

SettingScreenView::SettingScreenView() :
		configCallback(this, &SettingScreenView::configButtonPressed), configOKCallback(this, &SettingScreenView::configSet)
{
	baseview.addTo(&container);

	scrollableContainer1.setScrollThreshold(0);

	accordion.setXY(0, 0);
	accordion.setWidth(scrollableContainer1.getWidth());

	static const int menuHeight = 100;
	hc_menu = accordion.createMenu("Hand Control Configuration", menuHeight);
	mount_menu = accordion.createMenu("Mount Configuration", menuHeight);

	scrollableContainer1.add(accordion);

	num_config = 0;
}

SettingScreenView::~SettingScreenView()
{

}

void SettingScreenView::setupScreen()
{
	configPopup1.setVisible(false);

	// Setup menu structure
	num_config = presenter->getConfigAll(configs, MAX_CONFIG);

	debug("# of config: %d\r\n", num_config);

	for (int i = 0; i < num_config; i++)
	{
		abuttons[i] = new ButtonItem;
		if (!abuttons[i])
			break;

		int nameLen = strlen(configs[i].name) + 1;
		Unicode::UnicodeChar *nameBuf = new Unicode::UnicodeChar[nameLen];
		if (!nameBuf)
			break;
		Unicode::strncpy(nameBuf, configs[i].name, nameLen);
		abuttons[i]->setName(nameBuf);

		Unicode::UnicodeChar *valueBuf = new Unicode::UnicodeChar[32];
		if (!valueBuf)
			break;
		*valueBuf = 0;
		abuttons[i]->setValue(valueBuf);

		//Associate the config with the button for easier callback
		abuttons[i]->setUserData(&configs[i]);

		mount_menu->addItem(*abuttons[i], configCallback);
	}
}

void SettingScreenView::tearDownScreen()
{
	// Delete ALL allocated resources
	for (int i = 0; i < num_config; i++)
	{
		delete[] abuttons[i]->getName();
		delete[] abuttons[i]->getValue();
		delete abuttons[i];
	}
}

void SettingScreenView::configButtonPressed(const ButtonItem& button)
{
	ConfigItem *config = (ConfigItem *) button.getUserData();
	configPopup1.editConfig(config);
	configPopup1.setCallback(&configOKCallback);
}

void SettingScreenView::configSet(ConfigItem* config, bool ok)
{
	if (ok)
	{
		presenter->writeConfig(config);
	}
}
