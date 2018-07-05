#include <gui/alignscreen_screen/AlignScreenView.hpp>
#include <gui/alignscreen_screen/AlignScreenPresenter.hpp>

AlignScreenPresenter::AlignScreenPresenter(AlignScreenView& v) :
		BaseScreenPresenter(v.baseview), view(v)
{
}

void AlignScreenPresenter::activate()
{
	BaseScreenPresenter::activate();
}

void AlignScreenPresenter::deactivate()
{

}
