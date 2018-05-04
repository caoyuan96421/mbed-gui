#include <gui/mountscreen_screen/MountScreenView.hpp>
#include <gui/mountscreen_screen/MountScreenPresenter.hpp>

MountScreenPresenter::MountScreenPresenter(MountScreenView& v) :
		BaseScreenPresenter(v.baseview), view(v)
{
}

void MountScreenPresenter::activate()
{

}

void MountScreenPresenter::deactivate()
{

}
