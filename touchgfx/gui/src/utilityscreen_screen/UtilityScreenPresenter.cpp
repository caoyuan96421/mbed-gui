#include <gui/utilityscreen_screen/UtilityScreenView.hpp>
#include <gui/utilityscreen_screen/UtilityScreenPresenter.hpp>

UtilityScreenPresenter::UtilityScreenPresenter(UtilityScreenView& v) :
		BaseScreenPresenter(v.baseview), view(v)
{
}

void UtilityScreenPresenter::activate()
{
	BaseScreenPresenter::activate();
}

void UtilityScreenPresenter::deactivate()
{

}
