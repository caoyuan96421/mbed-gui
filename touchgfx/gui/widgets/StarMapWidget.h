/*
 * StarMapWidget.h
 *
 *  Created on: 2018��5��4��
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_
#define TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_

#include <touchgfx/widgets/canvas/CanvasWidget.hpp>
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>
#include <touchgfx/widgets/canvas/Canvas.hpp>
#include <touchgfx/Color.hpp>
#include "StarCatalog.h"
#include "mbed.h"

using namespace touchgfx;

class StarMapWidget: public CanvasWidget
{
public:
	StarMapWidget();
	virtual ~StarMapWidget()
	{
	}

	virtual bool drawCanvasWidget(const Rect& invalidatedArea) const;

	void aimAt(double ra, double dec)
	{
		ra_ctr = remainderf(ra, 360.0f);
		if (dec > 90.0f)
			dec_ctr = 90.0f;
		else if (dec < -90.0f)
			dec_ctr = -90.0f;
		else
			dec_ctr = dec;
		invalidate();
	}

	void setFOV(double fov)
	{
		if (fov > 45.0f)
			fov = 45.0f;
		if (fov < 0.5f)
		{
			fov = 0.5f;
		}
		fovw = fov;
		fovh = fov / getWidth() * getHeight();
		invalidate();
	}

	void setRotation(double r)
	{
		rot = r;
		invalidate();
	}

	double getFOV()
	{
		return fovw;
	}

	double getRotation()
	{
		return rot;
	}

	double getRA()
	{
		return ra_ctr;
	}

	double getDEC()
	{
		return dec_ctr;
	}

	void setColor(colortype c)
	{
		starpainter.setColor(c);
		invalidate();
	}

	colortype getColor()
	{
		return starpainter.getColor();
	}

	virtual void handleDragEvent(const DragEvent& evt);
	virtual void handleClickEvent(const ClickEvent& evt);

protected:
	float ra_ctr, dec_ctr; // Center of FOV
	float fovw; // Field of view along the width of the widget
	float fovh; // Field of view along the height of the widget
	float rot; // Rotation of FOV
	PainterRGB888 starpainter;

private:
	// These are for internal use
	mutable Canvas *canvas;
	mutable float xc, yc, zc;
	mutable float xp, yp, zp;
	mutable float xq, yq;
	mutable float fovr; // Field radius

	bool ispressed;
	bool isdoubleclick;
	bool isdrag;
	bool first;
	Timer tim;

	static void callback(StarItem*, void*);

};

#endif /* TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_ */
