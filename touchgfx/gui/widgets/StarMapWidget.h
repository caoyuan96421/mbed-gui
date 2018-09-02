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
#include <touchgfx/widgets/AbstractButton.hpp>
#include <touchgfx/Color.hpp>
#include "StarCatalog.h"
#include "mbed.h"
#include "PlanetMoon.h"

class StarMapWidget: public touchgfx::CanvasWidget
{
public:
	StarMapWidget();
	virtual ~StarMapWidget();

	virtual bool drawCanvasWidget(const touchgfx::Rect& invalidatedArea) const;
	virtual void draw(const touchgfx::Rect& invalidatedArea) const;

	void aimAt(double ra, double dec)
	{
		ra_ctr = remainder(ra, 360.0);
		if (dec > 90.0)
			dec_ctr = 90.0;
		else if (dec < -90.0)
			dec_ctr = -90.0;
		else
			dec_ctr = dec;
		updateView();
		invalidate();
	}

	void aimAt(const EquatorialCoordinates &eq)
	{
		aimAt(eq.ra, eq.dec);
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
		updateView();
		invalidate();
	}

	void setRotation(double r)
	{
		rot = r;
		updateView();
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

	void setColor(touchgfx::colortype c)
	{
		painter.setColor(c);
		invalidate();
	}

	touchgfx::colortype getColor()
	{
		return painter.getColor();
	}

	virtual void handleDragEvent(const touchgfx::DragEvent& evt);
	virtual void handleClickEvent(const touchgfx::ClickEvent& evt);
	virtual void handleTickEvent();
	virtual void handleGestureEvent(const touchgfx::GestureEvent& evt){
	}

	const LocationCoordinates& getLocation() const
	{
		return location;
	}

	void setLocation(const LocationCoordinates& location)
	{
		this->location = location;
		updateView();
		invalidate();
	}

	void setDrawConstell(bool drawConstell)
	{
		this->drawConstell = drawConstell;
		invalidate();
	}

	void setSelectionCallback(touchgfx::GenericCallback< const StarInfo *> &cb){
		selectionCallback = &cb;
	}

protected:
	double ra_ctr, dec_ctr; // Center of FOV
	float fovw; // Field of view along the width of the widget
	float fovh; // Field of view along the height of the widget
	float rot; // Rotation of FOV
	LocationCoordinates location;
	mutable touchgfx::PainterRGB888 painter;
	touchgfx::colortype labelColor;
	uint8_t labelAlpha;
	touchgfx::Bitmap moon_bitmap;
	touchgfx::GenericCallback<const StarInfo *> *selectionCallback;
	mutable class TextureMapperEx: public touchgfx::TextureMapper
	{
	public:
		void setParent(Drawable *p)
		{
			this->parent = p;
		}
	} moon_texture;

	mutable class BoxEx: public touchgfx::Box
	{
	public:
		void setParent(Drawable *p)
		{
			this->parent = p;
		}
	} bgBox;

	bool drawConstell;

	void updateView();

private:
	// These are for internal use
//	mutable touchgfx::Canvas *canvas;
	mutable float xc, yc, zc;
	mutable float xp, yp, zp;
	mutable float xq, yq;
	mutable float fovr; // Field radius
	mutable bool renderSuccessful;
	mutable bool displayMoon, displaySun;
	mutable struct
	{
		touchgfx::CWRUtil::Q5 x;
		touchgfx::CWRUtil::Q5 y;
		touchgfx::CWRUtil::Q5 size;
		int apex;
		int illumangle;
	} moonPos, sunPos;

	mutable int tick_rotation;
	mutable StarInfo *selected;

	bool ispressed;bool isdoubleclick;bool isdrag;bool first;
	int clickStartTime;
	int lastClickDuration;
	int clickX, clickY;
	mutable Timer tim;
	time_t timestamp;

	static const int STARMAP_WIDGET_MAX_LABEL = 20;
	static const int STARMAP_WIDGET_MAX_LABEL_LENGTH = 20;
	static const int STARMAP_WIDGET_MAX_STARS = 500;
	mutable struct
	{
		char label[STARMAP_WIDGET_MAX_LABEL_LENGTH];
		int x;
		int y;
	} starlabels[STARMAP_WIDGET_MAX_LABEL];
	mutable int num_label;
	mutable struct
	StarPos{
		touchgfx::CWRUtil::Q5 x;
		touchgfx::CWRUtil::Q5 y;
		touchgfx::CWRUtil::Q5 size;
		touchgfx::colortype color;
		const StarInfo *info;
	} visibleStars[STARMAP_WIDGET_MAX_STARS];
	mutable int num_stars;

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
	void _handleStars(const StarInfo *, bool hires) const;
	void _handleSun(const SolarSystemInfo *) const;
	void _handleMoon(const SolarSystemInfo *) const;

	bool _calcScreenPosition(const StarInfo *, bool isStar, touchgfx::CWRUtil::Q5 &xscr, touchgfx::CWRUtil::Q5 &yscr, float) const;

	void _drawstar(touchgfx::Canvas &canvas, StarPos &) const;
	void _drawsun(touchgfx::Canvas &canvas) const;
	void _drawmoon(touchgfx::Canvas &canvas) const;
	void _drawticks(touchgfx::Canvas &canvas, touchgfx::CWRUtil::Q5 x, touchgfx::CWRUtil::Q5 y, touchgfx::CWRUtil::Q5 r) const;
	void _drawcross(touchgfx::Canvas &canvas) const;
	void _drawconstell(const touchgfx::Rect &invalidatedArea) const;
	void _drawline(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const touchgfx::Rect &invalid, touchgfx::colortype color, uint8_t *fb) const;

};

#endif /* TOUCHGFX_GUI_WIDGETS_STARMAPWIDGET_H_ */
