/*
 * StarMapWidget.cpp
 *
 *  Created on: 2018Äê5ÔÂ4ÈÕ
 *      Author: caoyuan9642
 */

#include <StarMapWidget.h>
#include <touchgfx/Color.hpp>
#include <touchgfx/events/DragEvent.hpp>
#include <touchgfx/widgets/canvas/AbstractShape.hpp>
#include <cmath>
#include <cstdio>
#include "CelestialMath.h"

// Shape for Magnitude-0 star
static const AbstractShape::ShapePoint<float> starshape[9] =
{
{ 4.0f, 0 },
{ 1.0f, 1.0f },
{ 0, 4.0f },
{ -1.0f, 1.0f },
{ -4.0f, 0 },
{ -1.0f, -1.0f },
{ 0, -4.0f },
{ 1.0f, -1.0f },
{ 4.0f, 0 } };

static const AbstractShape::ShapePoint<float> starsimple[5] =
{
{ 4.0f, 0 },
{ 0, 4.0f },
{ -4.0f, 0 },
{ 0, -4.0f },
{ 4.0f, 0 } };

static const float RADIAN = 180.0 / M_PI;
static const float DEGREE = M_PI / 180.0;

static int count = 0;

StarMapWidget::StarMapWidget() :
		ra_ctr(0/*84.3f*/), dec_ctr(85.0/*-1.2f*/), fovw(20), fovh(20), rot(0), canvas(
		NULL), xc(0), yc(0), zc(0), xp(0), yp(0), zp(0), xq(0), yq(0), fovr(
				14.142f), ispressed(false), isdoubleclick(false), isdrag(false), first(
		true)
{
	this->setPainter(starpainter);
	this->setTouchable(true);
	starpainter.setColor(Color::getColorFrom24BitRGB(255, 255, 255));
	tim.start();
}

static const int DOUBLE_CLICK_THRESHOLD_MS = 500;

void StarMapWidget::handleDragEvent(const DragEvent& evt)
{
	float dx = -(float) evt.getDeltaX() / getWidth();
	float dy = -(float) evt.getDeltaY() / getHeight();

	if (!isdoubleclick)
	{
		// Pan
		float ra = this->ra_ctr - dx * fovw;
		float dec = this->dec_ctr - dy * fovh;
		this->aimAt(ra, dec);
	}
	else
	{
		// Zoom
		float fov = fovw * expf(-1.5*dy);
		this->setFOV(fov);
	}

	isdrag = true;
}

void StarMapWidget::handleClickEvent(const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::PRESSED)
	{
		ispressed = true;
		if (!first && !isdrag && tim.read_ms() < DOUBLE_CLICK_THRESHOLD_MS)
		{
			isdoubleclick = true;
		}
		isdrag = false;
	}
	else
	{
		ispressed = false;
		isdoubleclick = false;
		tim.reset();
	}
	first = false;
}

void StarMapWidget::callback(StarItem *star, void *arg)
{
//	printf("Star #%d, BF=%s, name=%s, RA=%f, DEC=%f, mag=%f\r\n", star->id,
//			star->BFname, star->name, star->RA, star->DEC, star->magnitude);
	StarMapWidget *pw = (StarMapWidget *) arg;
	count++;
	if (!pw)
		return;
	Canvas *canvas = pw->canvas;
	if (!canvas)
		return;

	float xs = cosf(star->DEC * DEGREE) * cosf(star->RA * DEGREE);
	float ys = cosf(star->DEC * DEGREE) * sinf(star->RA * DEGREE);
	float zs = sinf(star->DEC * DEGREE);

	float dotsc = pw->xc * xs + pw->yc * ys + pw->zc * zs; // Dot product
	float rf = acosf(dotsc) * RADIAN; // Angular distance to field center

//	printf("rf=%f\r\n", rf);
	if (rf > pw->fovr)
	{
		return;
	}

	// Projected onto a plane perpendicular to (xc,yc,zc)
	float xj = xs - dotsc * pw->xc;
	float yj = ys - dotsc * pw->yc;
	float zj = zs - dotsc * pw->zc;
	float ljm1 = 1.0f / sqrtf(xj * xj + yj * yj + zj * zj); // 1/Vector length, for normalization

	// Calculate angle between (xj, yj, zj) and (xp, yp, zp)
	float sinpq = (pw->xp * xj + pw->yp * yj + pw->zp * zj) * ljm1;
	float cospq = -(pw->xq * xj + pw->yq * yj) * ljm1;
	float thetapq = atan2f(sinpq, cospq) * RADIAN + pw->rot;

	// Coordinates in fraction of FOV
	float xf = (rf * cosf(thetapq * DEGREE)) / pw->fovw;
	float yf = (rf * sinf(thetapq * DEGREE)) / pw->fovh;

//	printf("xf=%f, yf=%f\r\n", xf, yf);

	// Check if it is in the FOV
	if (xf > 0.5f || xf < -0.5f || yf > 0.5f || yf < -0.5f)
		return;

	// Coordinate on screen (Widget coordinate)
	float xscr = (xf + 0.5) * pw->getWidth();
	float yscr = (0.5 - yf) * pw->getHeight();

	float factor = powf(0.1f, star->magnitude * 0.2f) / (pw->fovw / 25.0f); // Brightness proportional to area, normalized by fov at 15deg
	if (factor < 0.125f)
		factor = 0.125f;
	if (factor > 4.0f)
	{
		factor = 4.0f;
	}

	if (factor <= 0.5f)
	{
		canvas->moveTo(xscr + starsimple[0].x * factor,
				yscr + starsimple[0].y * factor);
		for (unsigned int i = 1; i < 5; i++)
		{
			canvas->lineTo(xscr + starsimple[i].x * factor,
					yscr + starsimple[i].y * factor);
		}
	}
	else
	{
		canvas->moveTo(xscr + starshape[0].x * factor,
				yscr + starshape[0].y * factor);
		for (unsigned int i = 1; i < 9; i++)
		{
			canvas->lineTo(xscr + starshape[i].x * factor,
					yscr + starshape[i].y * factor);
		}
	}

}

bool StarMapWidget::drawCanvasWidget(const Rect& invalidatedArea) const
{
	fovr = sqrtf(fovw * fovw + fovh * fovh) * 0.5f; // Field radius
	xc = cosf(dec_ctr * DEGREE) * cosf(ra_ctr * DEGREE);
	yc = cosf(dec_ctr * DEGREE) * sinf(ra_ctr * DEGREE);
	zc = sinf(dec_ctr * DEGREE);

	xp = -sinf(dec_ctr * DEGREE) * cosf(ra_ctr * DEGREE);
	yp = -sinf(dec_ctr * DEGREE) * sinf(ra_ctr * DEGREE);
	zp = cosf(dec_ctr * DEGREE);

	xq = -sinf(ra_ctr * DEGREE);
	yq = cosf(ra_ctr * DEGREE);

	Canvas canvas(this, invalidatedArea);
	this->canvas = &canvas;

	count = 0;
//	StarCatalog::getInstance().query_allconstellations(StarMapWidget::callback,
//			-180.0, 180.0, -90.0, 90.0, (void *) this);

	float maxmagnitude = 6.5 - 5 * log10f(fovw / 20.0f); // Estimate maximum magnitude for searching

	// Search for stars
	if (dec_ctr + fovr >= 85.0f)
	{
		// FOV close to north pole, search entire pole region
		StarCatalog::getInstance().query_all(StarMapWidget::callback, -180.0f,
				180.0f, dec_ctr - fovr, 90.0f, (void *) this, maxmagnitude);
	}
	else if (dec_ctr - fovr <= -85.0f)
	{
		// FOV close to south pole, search entire pole region
		StarCatalog::getInstance().query_all(StarMapWidget::callback, -180.0f,
				180.0f, -90.0f, dec_ctr + fovr, (void *) this, maxmagnitude);
	}
	else
	{
		// FOV is contained within poles.

		// Maximum RA range must be corrected
		float maxdec = (dec_ctr > 0) ? (dec_ctr + fovr) : -(dec_ctr - fovr);
		float delta_ra = atanf(
				sqrtf(1.0f + tanf(maxdec * DEGREE) * tanf(maxdec * DEGREE))
						* tanf(fovr * DEGREE)) * RADIAN;

		StarCatalog::getInstance().query_all(StarMapWidget::callback,
				remainderf(ra_ctr - delta_ra, 360.0f),
				remainderf(ra_ctr + delta_ra, 360.0f), dec_ctr - fovr,
				dec_ctr + fovr, (void *) this, maxmagnitude);
	}

	printf("count=%d\r\n", count);
	this->canvas = NULL;
	return canvas.render();
}
