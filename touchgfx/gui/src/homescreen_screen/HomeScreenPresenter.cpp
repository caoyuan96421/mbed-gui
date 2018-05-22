#include <gui/homescreen_screen/HomeScreenView.hpp>
#include <gui/homescreen_screen/HomeScreenPresenter.hpp>

HomeScreenPresenter::HomeScreenPresenter(HomeScreenView& v) :
		BaseScreenPresenter(v.baseview), view(v)
{
}

void HomeScreenPresenter::activate()
{
	BaseScreenPresenter::activate();
}

void HomeScreenPresenter::deactivate()
{

}

