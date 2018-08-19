#include <gui/polaralignscreen_screen/PolarAlignScreenView.hpp>
#include <gui/polaralignscreen_screen/PolarAlignScreenPresenter.hpp>

PolarAlignScreenPresenter::PolarAlignScreenPresenter(PolarAlignScreenView& v)
    : BaseScreenPresenter(v.baseview), view(v)
{
}

void PolarAlignScreenPresenter::activate()
{
	BaseScreenPresenter::activate();
}

void PolarAlignScreenPresenter::deactivate()
{

}
