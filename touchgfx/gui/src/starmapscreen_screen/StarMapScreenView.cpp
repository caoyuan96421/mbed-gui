#include <gui/starmapscreen_screen/StarMapScreenView.hpp>

StarMapScreenView::StarMapScreenView() :
		buttonCallback(this, &StarMapScreenView::buttonPressed)
{
	baseview.addTo(&container);

	starmap.setPosition(20, 20, 440, 440);

	add(starmap);

	button_zoomin.setAction(buttonCallback);
	button_zoomout.setAction(buttonCallback);

	button_left.setAction(buttonCallback);
	button_right.setAction(buttonCallback);
	button_down.setAction(buttonCallback);
	button_up.setAction(buttonCallback);
}

void StarMapScreenView::setupScreen()
{

}

void StarMapScreenView::tearDownScreen()
{

}

void StarMapScreenView::buttonPressed(const AbstractButton& src)
{
	float fov = starmap.getFOV();
	double ra = starmap.getRA();
	double dec = starmap.getDEC();

	if (&src == &button_zoomin || &src == &button_zoomout)
	{
		if (&src == &button_zoomout)
		{
			fov *= 1.25;
		}
		else
		{
			fov /= 1.25;
		}
		if (fov > 45)
		{
			// Max FOV: 45 deg
			fov = 45;
		}
		if (fov < 1)
		{
			// Min FOV: 1 deg
			fov = 1;
		}
		starmap.setFOV(fov);
	}
	else
	{
		if (&src == &button_right)
		{
			ra = remainder(ra - fov * 0.2, 360.0);
		}
		else if (&src == &button_left)
		{
			ra = remainder(ra + fov * 0.2, 360.0);
		}
		else if (&src == &button_down)
		{
			dec = dec + fov * 0.2;
			if (dec > 90)
				dec = 90;
		}
		else if (&src == &button_up)
		{
			dec = dec - fov * 0.2;
			if (dec < -90)
				dec = -90;
		}
		starmap.aimAt(ra, dec);
	}
}
