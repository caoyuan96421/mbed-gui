/*
 * StarMapWidget.cpp
 *
 *  Created on: 2018Äê5ÔÂ4ÈÕ
 *      Author: caoyuan9642
 */

#include <StarMapWidget.h>
#include <touchgfx/Color.hpp>
#include <touchgfx/widgets/canvas/AbstractShape.hpp>
#include <cmath>
#include <cstdio>
#include "CelestialMath.h"

// Shape for Magnitude-0 star
static const AbstractShape::ShapePoint<float> starshape[4] =
{
{ -4.0, 0 },
{ 0, 4.0 },
{ 4.0, 0 },
{ 0, -4.0 } };

static const float RADIAN = 180.0 / M_PI;
static const float DEGREE = M_PI / 180.0;

StarMapWidget::StarMapWidget() :
		ra_ctr(84.3), dec_ctr(-1.2), fovw(20), fovh(20), rot(0), canvas(NULL), xc(
				0), yc(0), zc(0), xp(0), yp(0), zp(0), xq(0), yq(0), fovr(
				14.142)
{
	this->setPainter(starpainter);
	starpainter.setColor(Color::getColorFrom24BitRGB(255, 255, 255));
}

void StarMapWidget::callback(StarItem *star, void *arg)
{
//	printf("Star #%d, BF=%s, name=%s, RA=%f, DEC=%f, mag=%f\r\n", star->id,
//			star->BFname, star->name, star->RA, star->DEC, star->magnitude);
	StarMapWidget *pw = (StarMapWidget *) arg;
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
	float ljm1 = 1 / sqrtf(xj * xj + yj * yj + zj * zj); // 1/Vector length, for normalization

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

	float factor = pow(0.1, star->magnitude * 0.2); // Brightness proportional to area
	if (factor < 0.125)
		factor = 0.125;

	canvas->moveTo(xscr + starshape[sizeof(star) - 1].x * factor,
			yscr + starshape[sizeof(star) - 1].y * factor);
	for (unsigned int i = 0; i < sizeof(star); i++)
	{
		canvas->lineTo(xscr + starshape[i].x * factor,
				yscr + starshape[i].y * factor);
	}

}

bool StarMapWidget::drawCanvasWidget(const Rect& invalidatedArea) const
{
	fovr = sqrt(fovw * fovw + fovh * fovh) * 0.5; // Field radius
	xc = cosf(dec_ctr * DEGREE) * cosf(ra_ctr * DEGREE);
	yc = cosf(dec_ctr * DEGREE) * sinf(ra_ctr * DEGREE);
	zc = sinf(dec_ctr * DEGREE);

	xp = sinf(dec_ctr * DEGREE) * cosf(ra_ctr * DEGREE);
	yp = sinf(dec_ctr * DEGREE) * sinf(ra_ctr * DEGREE);
	zp = cosf(dec_ctr * DEGREE);

	xq = -sinf(ra_ctr * DEGREE);
	yq = cosf(ra_ctr * DEGREE);

	Canvas canvas(this, invalidatedArea);
	this->canvas = &canvas;

	// Search for stars
	if (dec_ctr + fovr >= 90.0)
	{
		// FOV may intersect north pole, search entire pole region
		StarCatalog::getInstance().query_allconstellations(
				StarMapWidget::callback, -180.0, 180.0, dec_ctr - fovr, 90.0,
				(void *) this);
	}
	else if (dec_ctr - fovr <= -90.0)
	{
		// FOV may intersect south pole, search entire pole region
		StarCatalog::getInstance().query_allconstellations(
				StarMapWidget::callback, -180.0, 180.0, -90.0, dec_ctr + fovr,
				(void *) this);
	}
	else
	{
		// FOV is contained within poles.
		StarCatalog::getInstance().query_allconstellations(
				StarMapWidget::callback, remainder(ra_ctr - fovr, 360.0),
				remainder(ra_ctr + fovr, 360.0), dec_ctr - fovr, dec_ctr + fovr,
				(void *) this);
	}

	this->canvas = NULL;
	return canvas.render();
}
