#include <gui/settingscreen_screen/SettingScreenView.hpp>
#include "BitmapDatabase.hpp"
#include <cstdlib>
#include <cstring>

SettingScreenView::SettingScreenView()
{
	baseview.addTo(&container);

	scrollableContainer1.setScrollThreshold(1);

//	int y = 0;
//	int dy = 100;
//	cbn = 0;
//	while (strlen(TelescopeBackend::configlist[cbn].name) > 0)
//		cbn++;
//
//	typedef ConfigButton *pcb;
//	cbt = new pcb[cbn];
//
//	for (int i = 0; i < cbn; i++)
//	{
//		cbt[i] = new ConfigButton(TelescopeBackend::configlist[i],
//				configPopup1);
//		cbt[i]->setPosition(0, y, scrollableContainer1.getWidth(), dy);
//		y += dy;
//		scrollableContainer1.add(*cbt[i]);
//	}

}

SettingScreenView::~SettingScreenView()
{
//	for (int i = 0; i < cbn; i++)
//	{
//		delete cbt[i];
//	}
//	delete cbt;
}

void SettingScreenView::setupScreen()
{
	configPopup1.setVisible(false);
}

void SettingScreenView::tearDownScreen()
{

}
