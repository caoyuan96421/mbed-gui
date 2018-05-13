/*
 * StarMapWidget.h
 *
 *  Created on: 2018Äê5ÔÂ4ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_
#define TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_

#include <touchgfx/widgets/canvas/CanvasWidget.hpp>
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>
#include <touchgfx/widgets/canvas/Canvas.hpp>
#include <touchgfx/widgets/TextureMapper.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/Color.hpp>
#include "StarCatalog.h"
#include "mbed.h"
#include "PlanetMoon.h"

using namespace touchgfx;

class StarMapWidget: public CanvasWidget
{
public:
	StarMapWidget();
	virtual ~StarMapWidget();

	virtual bool drawCanvasWidget(const Rect& invalidatedArea) const;
	virtual void draw(const Rect& invalidatedArea) const;

	void aimAt(double ra, double dec)
	{
		ra_ctr = remainder(ra, 360.0);
		if (dec > 90.0)
			dec_ctr = 90.0;
		else if (dec < -90.0)
			dec_ctr = -90.0;
		else
			dec_ctr = dec;
		invalidate();
	}

	void setFOV(double fov)
	{
		if (fov > 45.0)
			fov = 45.0;
		if (fov < 0.5)
		{
			fov = 0.5;
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
	virtual void handleTickEvent();

	const LocationCoordinates& getLocation() const
	{
		return location;
	}

	void setLocation(const LocationCoordinates& location)
	{
		this->location = location;
	}

protected:
	double ra_ctr, dec_ctr; // Center of FOV
	float fovw; // Field of view along the width of the widget
	float fovh; // Field of view along the height of the widget
	float rot; // Rotation of FOV
	LocationCoordinates location;
	mutable PainterRGB888 starpainter;
	colortype labelColor;
	uint8_t labelAlpha;
	Bitmap moon_bitmap;
	mutable class TextureMapperEx: public TextureMapper
	{
	public:
		void setParent(Drawable *p)
		{
			this->parent = p;
		}
	} moon_texture;

	mutable class BoxEx: public Box
	{
	public:
		void setParent(Drawable *p)
		{
			this->parent = p;
		}
	} bgBox;

private:
	// These are for internal use
	mutable Canvas *canvas;
	mutable float xc, yc, zc;
	mutable float xp, yp, zp;
	mutable float xq, yq;
	mutable float fovr; // Field radius
	mutable bool renderSuccessful;
	mutable bool displayMoon;
	mutable CWRUtil::Q5 moon_x;
	mutable CWRUtil::Q5 moon_y;
	mutable CWRUtil::Q5 moon_size;
	mutable int moon_apex;
	mutable int moon_illumangle;
	mutable int tick_rotation;
	mutable StarInfo *selected;

	bool ispressed;bool isdoubleclick;bool isdrag;bool first;
	int clickStartTime;
	int lastClickDuration;
	int clickX, clickY;
	mutable Timer tim;
	time_t timestamp;

	static const unsigned int STARMAP_WIDGET_MAX_LABEL = 20;
	static const unsigned int STARMAP_WIDGET_MAX_LABEL_LENGTH = 20;
	mutable struct
	{
		char label[STARMAP_WIDGET_MAX_LABEL_LENGTH];
		int x;
		int y;
	} starlabels[STARMAP_WIDGET_MAX_LABEL];
	mutable unsigned int num_label;

	mutable struct SolarSystemInfo: StarInfo
	{
		MoonPhase phase;
		EquatorialCoordinatesWithDist accurate_pos;
		PlanetMoon::Object obj;

		virtual bool isPlanet() const
		{
			return (obj != PlanetMoon::SUN) && (obj != PlanetMoon::MOON);
		}
	} planetSunMoon[10];

	static void callback(StarInfo*, void*);
	void _callback(const StarInfo *, bool isStar) const;
	void _drawmoon() const;
	void _drawticks(CWRUtil::Q5 x, CWRUtil::Q5 y, CWRUtil::Q5 r) const;
	void _drawconstell(const Rect &invalidatedArea) const;
	void _drawline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Rect &invalid, colortype color, uint8_t *fb) const;

};

#endif /* TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_ */
