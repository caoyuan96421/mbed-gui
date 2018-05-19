#include <gui/mountscreen_screen/MountScreenView.hpp>
#include <gui/mountscreen_screen/MountScreenPresenter.hpp>

MountScreenPresenter::MountScreenPresenter(MountScreenView& v) :
		BaseScreenPresenter(v.baseview), view(v)
{
	view.setCoords(model->getEqCoords(), model->getMountCoords());
}

void MountScreenPresenter::activate()
{

}

void MountScreenPresenter::deactivate()
{

}

void MountScreenPresenter::setCoords(const EquatorialCoordinates& eq, const MountCoordinates& meq)
{
	BaseScreenPresenter::setCoords(eq, meq);
	view.setCoords(eq, meq);
}

