#ifndef SETTINGSCREEN_VIEW_HPP
#define SETTINGSCREEN_VIEW_HPP

#include <gui_generated/settingscreen_screen/SettingScreenViewBase.hpp>
#include <gui/settingscreen_screen/SettingScreenPresenter.hpp>
#include <gui/basescreen_screen/BaseScreenView.hpp>
#include <gui/widgets/ConfigButton.h>
#include <gui/BaseScreenAdaptor.h>
#include "TelescopeBackend.h"

class SettingScreenView : public SettingScreenViewBase, public BaseScreenAdaptor
{
public:
    SettingScreenView();
    virtual ~SettingScreenView();
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
//    ConfigButton **cbt;
//    int cbn;
};

#endif // SETTINGSCREEN_VIEW_HPP
