#include <gui/mountscreen_screen/MountScreenView.hpp>

MountScreenView::MountScreenView()
{
	baseview.addTo(&container);

	starmap.setPosition(starmap_bb.getX()+1, starmap_bb.getY()+1, starmap_bb.getWidth()-2, starmap_bb.getHeight()-2);
	add(starmap);

	starmap.setFOV(2);
	starmap.setTouchable(false);
}

void MountScreenView::setupScreen()
{

}

void MountScreenView::tearDownScreen()
{

}
