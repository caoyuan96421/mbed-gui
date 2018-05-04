#include <gui/settingscreen_screen/SettingScreenView.hpp>
#include <gui/settingscreen_screen/SettingScreenPresenter.hpp>

SettingScreenPresenter::SettingScreenPresenter(SettingScreenView& v) :
		BaseScreenPresenter(v.baseview), view(v)
{
}

void SettingScreenPresenter::activate()
{

}

void SettingScreenPresenter::deactivate()
{

}
