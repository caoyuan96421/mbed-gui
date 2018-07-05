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
#include <touchgfx/TypedText.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <BitmapDatabase.hpp>
#include <cmath>
#include <cstdio>
#include "SkyCulture.h"

using namespace touchgfx;

#define SMW_DEBUG 0

static const float RADIAN_F = 180.0f / M_PI;
static const float DEGREE_F = M_PI / 180.0f;
static const double RADIAN = 180.0 / M_PI;
static const double DEGREE = M_PI / 180.0;

static int count = 0;

static const int DRAG_THRESHOLD = 30;
static const int DOUBLE_CLICK_THRESHOLD_MS = 200;
static const int DOUBLE_CLICK_DURATION_MS = 200;
static const float SELECTION_ERROR_THRESHOLD = 50.0f;

static const float CONSTELLATION_LINE_RESOLUTION = 5.0f;

template<class T>
static inline T clip(T x)
{
	if (x > (T) 1.0)
		return (T) 1.0;
	else if (x < (T) -1.0)
		return -(T) 1.0;
	else
		return x;
}

StarMapWidget::StarMapWidget() :
		ra_ctr(84.3f), dec_ctr(-1.2f), fovw(20), fovh(20), rot(0), location(42, -73), moon_bitmap(BITMAP_MOON_SMALL_ID), canvas(
		NULL), xc(0), yc(0), zc(0), xp(0), yp(0), zp(0), xq(0), yq(0), fovr(14.142f), renderSuccessful(true), displayMoon(false), moon_apex(0), moon_illumangle(0), tick_rotation(0), selected(NULL), ispressed(
		false), isdoubleclick(
		false), isdrag(false), first(
		true), clickStartTime(0), lastClickDuration(0), clickX(-1000), clickY(-1000), timestamp(time(NULL)), num_label(0)
{
	this->setPainter(starpainter);
	this->setTouchable(true);
	starpainter.setColor(Color::getColorFrom24BitRGB(255, 255, 255), 255);
	labelColor = Color::getColorFrom24BitRGB(200, 200, 200);
	labelAlpha = 200;
	tim.start();
	moon_texture.setBitmap(moon_bitmap);
	moon_texture.setPosition(0, 0, 300, 300);
	moon_texture.setParent(this);
	bgBox.setParent(this);
	bgBox.setColor(Color::getColorFrom24BitRGB(20, 20, 20));

	for (PlanetMoon::Object obj = PlanetMoon::MERCURY; obj <= PlanetMoon::MOON; obj = (PlanetMoon::Object) (obj + 1))
	{
		strncpy(planetSunMoon[(int) obj].name, PlanetMoon::getName(obj), sizeof(planetSunMoon[(int) obj].name));
		planetSunMoon[(int) obj].color = PlanetMoon::getColor(obj);
		planetSunMoon[(int) obj].magnitude = PlanetMoon::getMagnitude(obj);
		planetSunMoon[(int) obj].obj = obj;
	}

	Application::getInstance()->registerTimerWidget(this);
}

StarMapWidget::~StarMapWidget()
{
	Application::getInstance()->unregisterTimerWidget(this);
}

void StarMapWidget::handleDragEvent(const DragEvent& evt)
{
	float dx = -(float) evt.getDeltaX() / getWidth();
	float dy = -(float) evt.getDeltaY() / getHeight();

	int xx = evt.getNewX() - clickX; // Total amount moved
	int yy = evt.getNewY() - clickY;

	if (isdrag || xx * xx + yy * yy >= DRAG_THRESHOLD * DRAG_THRESHOLD)
	{
		if (!isdoubleclick)
		{
			// Pan
			float ra_factor = 1.0f / (cosf(fabsf(this->dec_ctr) * DEGREE_F) + 1e-9);
			if (fabsf(this->dec_ctr) == 90.0 || ra_factor > 3)
				ra_factor = 3;
			float ra = this->ra_ctr - dx * fovw * ra_factor;
			float dec = this->dec_ctr - dy * fovh;
			this->aimAt(ra, dec);
		}
		else
		{
			// Zoom
			float fov = fovw * expf(-2.5f * dy);
			this->setFOV(fov);
		}
		isdrag = true;
	}

}

void StarMapWidget::handleClickEvent(const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::PRESSED)
	{
		// Press
		ispressed = true;
		if (!first && !isdrag && tim.read_ms() < DOUBLE_CLICK_THRESHOLD_MS && lastClickDuration < DOUBLE_CLICK_DURATION_MS)
		{
			isdoubleclick = true;
		}
		isdrag = false;
		clickStartTime = tim.read_ms();
		clickX = evt.getX();
		clickY = evt.getY();
	}
	else
	{
		// Release
		if (!isdrag)
		{
			// Find the star being selected
			float x_delta = ((float) evt.getX() / getWidth() - 0.5f) * fovw;
			float y_delta = ((float) evt.getY() / getHeight() - 0.5f) * fovh;
			debug_if(SMW_DEBUG, "Selected X=%f, Y=%f\r\n", x_delta, y_delta);

			// Field radius in degree
			float rf = sqrtf(x_delta * x_delta + y_delta * y_delta);
			float theta = (90.0f + rot) * DEGREE_F + atan2f(y_delta, x_delta);
			debug_if(SMW_DEBUG, "Selected R=%f, Th=%f\r\n", rf, theta);

			float p = cosf(rf * DEGREE_F);
			float q = sqrtf(1.0f - p * p);

			// Selected position on equatorial sphere
			float xs = p * xc + q * (-cosf(theta) * sinf(dec_ctr * DEGREE_F) * cosf(ra_ctr * DEGREE_F) + sinf(theta) * sinf(ra_ctr * DEGREE_F));
			float ys = p * yc + q * (-cosf(theta) * sinf(dec_ctr * DEGREE_F) * sinf(ra_ctr * DEGREE_F) - sinf(theta) * cosf(ra_ctr * DEGREE_F));
			float zs = p * zc + q * (cosf(theta) * cosf(dec_ctr * DEGREE_F));

			float ra = atan2f(ys, xs) * RADIAN_F;
			float dec = atan2f(zs, sqrtf(xs * xs + ys * ys)) * RADIAN_F;
			float maxdist = SELECTION_ERROR_THRESHOLD / getWidth() * fovw;

			debug_if(SMW_DEBUG, "Selected RA=%f, DEC=%f, maxdist=%f\r\n", ra, dec, maxdist);

			StarInfo *newselected = NULL;
			// First check planets and Sun/Moon
			for (PlanetMoon::Object obj = PlanetMoon::MERCURY; obj <= PlanetMoon::MOON; obj = (PlanetMoon::Object) (obj + 1))
			{
				float dr = planetSunMoon[(int) obj].RA - ra;
				float dd = planetSunMoon[(int) obj].DEC - dec;
				if (dr * dr + dd * dd < maxdist * maxdist)
				{
					newselected = &planetSunMoon[(int) obj];
					break;
				}
			}

			// Then check stars
			if (!newselected)
			{
				newselected = StarCatalog::getInstance().searchByCoordinates(ra, dec, maxdist);
			}

			if (selected != newselected)
			{
				debug_if(SMW_DEBUG, "Selected star: id=%d, ra=%f, dec=%f\r\n", selected->id, selected->RA, selected->DEC);
				selected = newselected;
				invalidate();
			}
		}

		ispressed = false;
		isdoubleclick = false;
		lastClickDuration = tim.read_ms() - clickStartTime;
		tim.reset();
	}
	first = false;
}

void StarMapWidget::draw(const Rect& invalidatedArea) const
{
	uint64_t time_start = tim.read_high_resolution_us();
// Update parameters
	fovr = sqrtf(fovw * fovw + fovh * fovh) * 0.5f; // Field radius
	// Field center
	xc = cosf(dec_ctr * DEGREE_F) * cosf(ra_ctr * DEGREE_F);
	yc = cosf(dec_ctr * DEGREE_F) * sinf(ra_ctr * DEGREE_F);
	zc = sinf(dec_ctr * DEGREE_F);

	// Auxiliary vectors (both perpendicular to (xc,yc,zc) and to each other)
	xp = -sinf(dec_ctr * DEGREE_F) * cosf(ra_ctr * DEGREE_F);
	yp = -sinf(dec_ctr * DEGREE_F) * sinf(ra_ctr * DEGREE_F);
	zp = cosf(dec_ctr * DEGREE_F);

	xq = -sinf(ra_ctr * DEGREE_F);
	yq = cosf(ra_ctr * DEGREE_F);
// Draw BG box first
// Draw texture of moon
	bgBox.setPosition(0, 0, getWidth(), getHeight());
	Rect d = invalidatedArea & bgBox.getRect();
	if (!d.isEmpty())
	{
		d.x -= bgBox.getX();
		d.y -= bgBox.getY();
		bgBox.draw(d);
	}

	// Draw Constellations
	_drawconstell(invalidatedArea);

	num_label = 0;

	// Before drawing everything else, let's check moon
	SolarSystemInfo &moon = planetSunMoon[(int) PlanetMoon::MOON];
	EquatorialCoordinatesWithDist eq = PlanetMoon::calculatePosition(PlanetMoon::MOON, timestamp, location);
	moon.RA = eq.ra;
	moon.DEC = eq.dec;
	moon.accurate_pos = eq;
	moon.phase = PlanetMoon::getLunarPhase(timestamp, location, eq);
	displayMoon = false;
	_callback(&moon, false);

// Display the texture now if necessary
	if (displayMoon)
	{
		// Set moon position
		moon_texture.setXY((moon_x - moon_size).to<int>(), (moon_y - moon_size).to<int>());
		moon_texture.setWidth((moon_size * 2).to<int>());
		moon_texture.setHeight((moon_size * 2).to<int>());
		moon_texture.setScale((moon_size * 2).to<float>() / (moon_bitmap.getWidth() + 4));
		moon_texture.setBitmapPosition(2, 2);
		moon_texture.setAlpha(127);
		// Draw texture of moon
		d = invalidatedArea & moon_texture.getRect();
		if (!d.isEmpty())
		{
			d.x -= moon_texture.getX();
			d.y -= moon_texture.getY();
			moon_texture.draw(d);
		}
	}

// Now draw the star map, including the moon shadow
	CanvasWidget::draw(invalidatedArea);

// Now draw the label
	if (num_label > 0)
	{
		TypedText typedText(T_TINY);
		const unsigned int BUF_SIZE = 20;
		Unicode::UnicodeChar buf[BUF_SIZE];

		const Font* fontToDraw = typedText.getFont();
		if ((fontToDraw != 0) && typedText.hasValidId())
		{
			for (unsigned int i = 0; i < num_label; i++)
			{
				int16_t offsetx, offsety;
				Rect labelRect;
				offsetx = starlabels[i].x;
				offsety = starlabels[i].y;

				Unicode::strncpy(buf, starlabels[i].label, BUF_SIZE);
				buf[BUF_SIZE - 1] = 0; // Ensure termination

				uint8_t height = fontToDraw->getMinimumTextHeight();
				uint16_t width = fontToDraw->getStringWidth(buf);

				labelRect = Rect(offsetx, offsety - height / 2, width, height);
				Rect dirty = labelRect & invalidatedArea;

				if (!dirty.isEmpty())
				{
					dirty.x -= labelRect.x;
					dirty.y -= labelRect.y;
					translateRectToAbsolute(labelRect);
					LCD::StringVisuals visuals(fontToDraw, labelColor, labelAlpha, typedText.getAlignment(), 0, TEXT_ROTATE_0, typedText.getTextDirection(), 0, WIDE_TEXT_NONE);
					HAL::lcd().drawString(labelRect, dirty, visuals, buf);
				}
			}
		}
	}

	debug_if(SMW_DEBUG, "%lld us\r\n", tim.read_high_resolution_us() - time_start);
}

void StarMapWidget::handleTickEvent()
{
	static int count = 0;
	time_t newtime = time(NULL);
	if (newtime != timestamp)
	{
		// Happens every second
		timestamp = newtime;
//		timestamp = 1712604420; // 2024/04/08 19:27:00 (UTC), Solar eclipse
//		debug_if(SMW_DEBUG, "tick\r\n");
		invalidate();
	}
	if (++count == 20)
	{
		// Animate the tick rotation
		tick_rotation += 15;
		if (selected)
		{
			invalidate();
		}
		count = 0;
	}
}

void StarMapWidget::callback(StarInfo *star, void *arg)
{
	StarMapWidget *pw = (StarMapWidget *) arg;
	count++;
	if (!pw)
		return;

	pw->_callback(star, true);
}

void StarMapWidget::_callback(const StarInfo *star, bool isStar) const
{
	CWRUtil::Q5 xscr, yscr; // Screen coordinate of the object center

	if (isStar)
	{
		// Use precalculated float values for faster calculation
		float xs = cosf(star->DEC * DEGREE_F) * cosf(star->RA * DEGREE_F);
		float ys = cosf(star->DEC * DEGREE_F) * sinf(star->RA * DEGREE_F);
		float zs = sinf(star->DEC * DEGREE_F);

		float dotsc = xc * xs + yc * ys + zc * zs; // Dot product
		float rf = acosf(clip(dotsc)) * RADIAN_F; // Angular distance to field center

		if (rf > fovr)
		{
			return;
		}

		// Projected onto a plane perpendicular to (xc,yc,zc)
		float xj = xs - dotsc * xc;
		float yj = ys - dotsc * yc;
		float zj = zs - dotsc * zc;
		float ljm1 = 1.0f / sqrtf(xj * xj + yj * yj + zj * zj); // 1/Vector length, for normalization

		// Calculate angle between (xj, yj, zj) and (xp, yp, zp)
		float sinpq = (xp * xj + yp * yj + zp * zj) * ljm1;
		float cospq = -(xq * xj + yq * yj) * ljm1;
		float thetapq = atan2f(sinpq, cospq) * RADIAN_F + rot;

		// Coordinates in fraction of FOV
		float xf = (rf * cosf(thetapq * DEGREE_F)) / fovw;
		float yf = (rf * sinf(thetapq * DEGREE_F)) / fovh;
		// Check if it is in the FOV
		if (xf > 0.5f || xf < -0.5f || yf > 0.5f || yf < -0.5f)
			return;
		// Coordinate on screen (Widget coordinate)
		xscr = CWRUtil::toQ5((xf + 0.5f) * getWidth());
		yscr = CWRUtil::toQ5((0.5f - yf) * getHeight());
	}
	else
	{
		// Use high resolution coordinates (double)
		const SolarSystemInfo &ss = *((SolarSystemInfo *) star);

		// For planet and Sun/Moon, use high resolution coordinates
		double xs = cos(ss.accurate_pos.dec * DEGREE) * cos(ss.accurate_pos.ra * DEGREE);
		double ys = cos(ss.accurate_pos.dec * DEGREE) * sin(ss.accurate_pos.ra * DEGREE);
		double zs = sin(ss.accurate_pos.dec * DEGREE);

		double xc = cos(dec_ctr * DEGREE) * cos(ra_ctr * DEGREE);
		double yc = cos(dec_ctr * DEGREE) * sin(ra_ctr * DEGREE);
		double zc = sin(dec_ctr * DEGREE);

		double rf = acos(clip(xc * xs + yc * ys + zc * zs)) * RADIAN; // Angular distance to field center

		double Y = -cos(ss.accurate_pos.dec * DEGREE) * sin((ss.accurate_pos.ra - ra_ctr) * DEGREE);
		double X = -sin(dec_ctr * DEGREE) * cos(ss.accurate_pos.dec * DEGREE) * cos((ss.accurate_pos.ra - ra_ctr) * DEGREE) + cos(dec_ctr * DEGREE) * sin(ss.accurate_pos.dec * DEGREE);
		double thetapq = 90.0 - atan2(Y, X) * RADIAN + rot;

		double xf = (rf * cos(thetapq * DEGREE)) / fovw;
		double yf = (rf * sin(thetapq * DEGREE)) / fovh;
		// Check if it is in the FOV
		if (xf > 0.5 || xf < -0.5 || yf > 0.5 || yf < -0.5)
			return;
		// Coordinate on screen (Widget coordinate)
		xscr = CWRUtil::toQ5((xf + 0.5) * getWidth());
		yscr = CWRUtil::toQ5((0.5 - yf) * getHeight());
	}

//	debug_if(SMW_DEBUG, "xf=%f, yf=%f\r\n", xf, yf);

	float red = 1.0f + 0.10f * star->color;
	float green = 1.0f;
	float blue = 1.0f - 0.15f * star->color;
	float norm = sqrtf(red * red + green * green + blue * blue);

	colortype starColor = Color::getColorFrom24BitRGB(uint8_t(red / norm * 255.0f), uint8_t(green / norm * 255.0f), uint8_t(blue / norm * 255.0f));

	starpainter.setColor(starColor, 255);

	int label_x, label_y;
	CWRUtil::Q5 outer_radius;
	bool isMoon = false;

	if (isStar || ((const SolarSystemInfo *) star)->isPlanet() || (((const SolarSystemInfo *) star)->obj == PlanetMoon::SUN && fovw >= 20.0f))
	{
		if (!canvas)
			return;
		float size = powf(0.1f, star->magnitude * 0.2f) / fovw * 150.0f; // Brightness proportional to area, normalized by fov at 15deg
		if (size < 0.5f)
			size = 0.5f;
		if (size > 8.0f)
		{
			size = 8.0f;
		}

		CWRUtil::Q5 sq = CWRUtil::toQ5(size);

		int numstep = (size >= 1.0f) ? 12 : 4;
		int step = 360 / numstep;

		canvas->moveTo(xscr, yscr - sq);
		for (int i = 0; i <= 360; i += step)
		{
			canvas->lineTo(xscr + sq * CWRUtil::sine(i), yscr + sq * CWRUtil::cosine(i));
		}

		label_x = (int) ((xscr + sq).to<int>() + 2);
		label_y = (int) (yscr.to<int>() + 2);
		outer_radius = sq;
	}
	else
	{
		const SolarSystemInfo &ss = *((const SolarSystemInfo *) star);
		if (ss.obj == PlanetMoon::MOON)
		{
			// Moon
			CWRUtil::Q5 size = CWRUtil::toQ5(getWidth() / fovw * 0.25f);
			MoonPhase &phase = ((SolarSystemInfo *) star)->phase;

			debug_if(SMW_DEBUG, "Update moon: %d %d\r\n", xscr.to<int>(), yscr.to<int>());

			isMoon = true;
			displayMoon = true;
			moon_x = xscr;
			moon_y = yscr;
			moon_size = size;
			moon_apex = (int) (phase.illumaxis - rot);
			moon_illumangle = (int) phase.illumangle;

			//Actual drawing is delayed

			label_x = (int) ((xscr + size).to<int>() + 2);
			label_y = (int) (yscr.to<int>() + 2);
			outer_radius = size;
		}
		else
		{
			if (!canvas)
				return;
			// Sun
			CWRUtil::Q5 size = CWRUtil::toQ5(getWidth() / fovw * 0.25f);

			canvas->moveTo(xscr, yscr - size);
			for (int i = 0; i <= 360; i += 10)
			{
				canvas->lineTo(xscr + ((size * CWRUtil::sine(i))), yscr - ((size * CWRUtil::cosine(i))));
			}

			if (!canvas->render())
			{
				renderSuccessful = false;
				debug_if(SMW_DEBUG, "Render failed\r\n");
			}

			CWRUtil::Q5 size2 = size * 4;
			starpainter.setColor(Color::getColorFrom24BitRGB(255, 255, 100), 60);
			canvas->moveTo(xscr, yscr - size);
			for (int i = 0; i <= 360; i += 20)
			{
				canvas->lineTo(xscr + ((size * CWRUtil::sine(i))), yscr - ((size * CWRUtil::cosine(i))));
				canvas->lineTo(xscr + ((size2 * CWRUtil::sine(i + 10))), yscr - ((size2 * CWRUtil::cosine(i + 10))));
			}

			label_x = (int) ((xscr + size2).to<int>() + 2);
			label_y = (int) (yscr.to<int>() + 2);
			outer_radius = size2;
		}
	}

	if (*star->name != '\0' && num_label < STARMAP_WIDGET_MAX_LABEL)
	{
		// Generate name label
		strncpy(starlabels[num_label].label, star->name, STARMAP_WIDGET_MAX_LABEL_LENGTH);
		starlabels[num_label].x = label_x;
		starlabels[num_label].y = label_y;
		num_label++;
	}

	if (!isMoon)
		if (!canvas->render())
		{
			renderSuccessful = false;
			debug_if(SMW_DEBUG, "Render failed\r\n");
		}

	// Draw ticks around selected star
	if (!isMoon && selected && selected == star)
	{
		_drawticks(xscr, yscr, outer_radius);
	}
}

bool StarMapWidget::drawCanvasWidget(const Rect& invalidatedArea) const
{
	Canvas canvas(this, invalidatedArea);
	this->canvas = &canvas;

	count = 0;
	renderSuccessful = true;

	float maxmagnitude = 6.5 - 5 * log10f(fovw / 20.0f); // Estimate maximum magnitude for searching

	// Search for stars
	if (dec_ctr + fovr >= 85.0f)
	{
		// FOV close to north pole, search entire pole region
		StarCatalog::getInstance().query_all(StarMapWidget::callback, -180.0f, 180.0f, dec_ctr - fovr, 90.0f, (void *) this, maxmagnitude);
	}
	else if (dec_ctr - fovr <= -85.0f)
	{
		// FOV close to south pole, search entire pole region
		StarCatalog::getInstance().query_all(StarMapWidget::callback, -180.0f, 180.0f, -90.0f, dec_ctr + fovr, (void *) this, maxmagnitude);
	}
	else
	{
		// FOV is contained within poles.

		// Maximum RA range must be corrected
		float maxdec = (dec_ctr > 0) ? (dec_ctr + fovr) : -(dec_ctr - fovr);
		float delta_ra = atanf(sqrtf(1.0f + tanf(maxdec * DEGREE_F) * tanf(maxdec * DEGREE_F)) * tanf(fovr * DEGREE_F)) * RADIAN_F;

		StarCatalog::getInstance().query_all(StarMapWidget::callback, remainderf(ra_ctr - delta_ra, 360.0f), remainderf(ra_ctr + delta_ra, 360.0f), dec_ctr - fovr, dec_ctr + fovr, (void *) this,
				maxmagnitude);
	}

	// Deal with planets as if they are stars
	for (PlanetMoon::Object obj = PlanetMoon::MERCURY; obj <= PlanetMoon::SUN; obj = (PlanetMoon::Object) (obj + 1))
	{
		EquatorialCoordinatesWithDist eq = PlanetMoon::calculatePosition(obj, timestamp, location);
		planetSunMoon[(int) obj].RA = eq.ra;
		planetSunMoon[(int) obj].DEC = eq.dec;
		planetSunMoon[(int) obj].accurate_pos = eq;
		if (planetSunMoon[(int) obj].magnitude <= maxmagnitude)
			_callback(&planetSunMoon[(int) obj], false);
	}

	// Draw the moon shadow if necessary
	if (displayMoon)
	{
		_drawmoon();
	}

	this->canvas = NULL;
	return renderSuccessful;
}

void StarMapWidget::_drawmoon() const
{

	starpainter.setColor(bgBox.getColor(), 230); // BG color

	const int di = 5;
	CWRUtil::Q15 sina = CWRUtil::sine(moon_apex);
	CWRUtil::Q15 cosa = CWRUtil::cosine(moon_apex);

// Draw outline of half-circle
	canvas->moveTo(moon_x + moon_size * CWRUtil::sine(moon_apex), moon_y - moon_size * CWRUtil::cosine(moon_apex));
	for (int i = 180; i <= 360; i += di)
	{
		canvas->lineTo(moon_x + moon_size * CWRUtil::sine(i + moon_apex), moon_y - moon_size * CWRUtil::cosine(i + moon_apex));
	}

// Draw the dark fraction
	for (int i = 0; i <= 180; i += di)
	{
		CWRUtil::Q5 dx = (moon_size * CWRUtil::sine(i)) * CWRUtil::cosine(moon_illumangle);
		CWRUtil::Q5 dy = moon_size * CWRUtil::cosine(i);

		canvas->lineTo(moon_x + dx * cosa + dy * sina, moon_y + dx * sina - dy * cosa);
	}

	if (!canvas->render())
	{
		debug_if(SMW_DEBUG, "Render failed.\r\n");
		renderSuccessful = false;
	}

	if (selected == &planetSunMoon[(int) PlanetMoon::MOON])
	{
		// Moon selected
		_drawticks(moon_x, moon_y, moon_size);
	}
}

void StarMapWidget::_drawticks(CWRUtil::Q5 x0, CWRUtil::Q5 y0, CWRUtil::Q5 r) const
{
	const int ticks_spacing = 90; // 4 ticks in total
	if (!canvas)
	{
		return;
	}
	int angle = tick_rotation;

	starpainter.setColor(Color::getColorFrom24BitRGB(255, 200, 0), 200);
	CWRUtil::Q5 r1 = r + CWRUtil::toQ5(2);
	CWRUtil::Q5 r2 = r1 + CWRUtil::toQ5(10);
	CWRUtil::Q5 hw = CWRUtil::toQ5(0.5f); // Half of linewidth

	for (int i = 0; i < 4; i++, angle += ticks_spacing)
	{
		CWRUtil::Q15 s = CWRUtil::sine(angle);
		CWRUtil::Q15 c = CWRUtil::cosine(angle);
		canvas->moveTo(x0 + r1 * s + hw * c, y0 - r1 * c + hw * s);
		canvas->lineTo(x0 + r2 * s + hw * c, y0 - r2 * c + hw * s);
		canvas->lineTo(x0 + r2 * s - hw * c, y0 - r2 * c - hw * s);
		canvas->lineTo(x0 + r1 * s - hw * c, y0 - r1 * c - hw * s);
		canvas->lineTo(x0 + r1 * s + hw * c, y0 - r1 * c + hw * s);
	}

	if (!canvas->render())
	{
		debug_if(SMW_DEBUG, "Render failed.\r\n");
		renderSuccessful = false;
	}
}

void StarMapWidget::_drawconstell(const Rect &invalidatedArea) const
{
	SkyCulture::Iterator it;
	SkyCulture::Segment *sg;
	SkyCulture::resetIterator(it);
	Rect r(0, 0, getWidth(), getHeight());
	float stepangle = fovw / CONSTELLATION_LINE_RESOLUTION;
	int count = 0;
	int sum = 0;
	int timestart = tim.read_us();
//	starpainter.setColor(Color::getColorFrom24BitRGB(0, 75, 165), 180);
	uint8_t *fbuf = (uint8_t*) HAL::getInstance()->lockFrameBuffer();
	while ((sg = SkyCulture::nextSegment(it)) != NULL)
	{
		float &x1 = sg->x1;
		float &y1 = sg->y1;
		float &z1 = sg->z1;
		float &x2 = sg->x2;
		float &y2 = sg->y2;
		float &z2 = sg->z2;

		// Cross product, giving the axis of the great circle connecting the two stars
		float xg = y1 * z2 - z1 * y2;
		float yg = z1 * x2 - x1 * z2;
		float zg = x1 * y2 - y1 * x2;
		float l1rg = 1.0f / sqrtf(xg * xg + yg * yg + zg * zg);
		xg *= l1rg;
		yg *= l1rg;
		zg *= l1rg;

		//Check min distance to the field
		float dotcg = (xc * xg + yc * yg + zc * zg);
		float fdist = acosf(clip(dotcg)) * RADIAN_F;
		if (fabsf(fdist - 90.0f) > fovr)
		{
			continue;
		}

		// projection of field center to the great circle plane
		float xj = xc - dotcg * xg;
		float yj = yc - dotcg * yg;
		float zj = zc - dotcg * zg;
		float rj = sqrtf(xj * xj + yj * yj + zj * zj);

		// Check whether the projection lies in between star1 and star2
		float cp1 = xg * (y1 * zj - z1 * yj) + yg * (z1 * xj - x1 * zj) + zg * (x1 * yj - y1 * xj); // Sin of angle between proj and star1
		float cp2 = xg * (y2 * zj - z2 * yj) + yg * (z2 * xj - x2 * zj) + zg * (x2 * yj - y2 * xj); // Sin of angle between proj and star2
		float ang1 = asinf(clip(cp1 / rj)) * RADIAN_F;
		float ang2 = asinf(clip(cp2 / rj)) * RADIAN_F;
		if (cp1 * cp2 > 0)
		{
			// Not lying between star1 and star2, needs further check
			if (fabsf(ang1) >= fovr && fabsf(ang2) >= fovr)
			{
				// Out of range
				continue;
			}
		}
		float x, y, z;

		float ang12 = (ang1 - ang2);
		int nstep = abs((int) (ang12 / stepangle));
		if (nstep < 2)
		{
			// Skip really short lines
			nstep = 2;
		}

		float stepsize = ang12 / nstep;

		if (fabsf(ang1) < fabsf(ang2))
		{
			x = x1;
			y = y1;
			z = z1;
		}
		else
		{
			x = x2;
			y = y2;
			z = z2;
			stepsize = -stepsize;
		}

		stepsize *= DEGREE_F; // Convert to radian
		float cs = cosf(stepsize);
		float ss = sinf(stepsize);
		float a11 = (cs + xg * xg * (1 - cs));
		float a12 = (xg * yg * (1 - cs) - zg * ss);
		float a13 = (xg * zg * (1 - cs) + yg * ss);
		float a21 = (xg * yg * (1 - cs) + zg * ss);
		float a22 = (cs + yg * yg * (1 - cs));
		float a23 = (yg * zg * (1 - cs) - xg * ss);
		float a31 = (xg * zg * (1 - cs) - yg * ss);
		float a32 = (yg * zg * (1 - cs) + xg * ss);
		float a33 = (cs + zg * zg * (1 - cs));

		float xscr, yscr, x0 = -10000, y0 = -10000;

		for (int i = 0; i <= nstep; i++)
		{
			// Convert to screen coordinates
			float dotsc = xc * x + yc * y + zc * z; // Dot product
			float rf = acosf(clip(dotsc)) * RADIAN_F; // Angular distance to field center

			// Projected onto a plane perpendicular to (xc,yc,zc)
			float xj = x - dotsc * xc;
			float yj = y - dotsc * yc;
			float zj = z - dotsc * zc;
			float ljm1 = 1.0f / sqrtf(xj * xj + yj * yj + zj * zj); // 1/Vector length, for normalization

			// Calculate angle between (xj, yj, zj) and (xp, yp, zp)
			float sinpq = (xp * xj + yp * yj + zp * zj) * ljm1;
			float cospq = -(xq * xj + yq * yj) * ljm1;
			float thetapq = atan2f(sinpq, cospq) * RADIAN_F + rot;

			// Coordinates in fraction of FOV
			float xf = (rf * cosf(thetapq * DEGREE_F)) / fovw;
			float yf = (rf * sinf(thetapq * DEGREE_F)) / fovh;

			// Coordinate on screen (Widget coordinate)
			xscr = (xf + 0.5f) * getWidth();
			yscr = (0.5f - yf) * getHeight();

			if (i != 0 && (r.intersect(xscr, yscr) || r.intersect(x0, y0)))
			{
				// Draw a line from x0, y0 to xscr, yscr
				_drawline((int) x0, (int) y0, (int) xscr, (int) yscr, invalidatedArea, Color::getColorFrom24BitRGB(160, 85, 0), fbuf);
			}

			x0 = xscr;
			y0 = yscr;

			// Rotation around (xg, yg, zg) for stepsize
			float xr = a11 * x + a12 * y + a13 * z;
			float yr = a21 * x + a22 * y + a23 * z;
			float zr = a31 * x + a32 * y + a33 * z;
			x = xr;
			y = yr;
			z = zr;

		}
	}
	HAL::getInstance()->unlockFrameBuffer();
	sum += tim.read_us() - timestart;

	debug_if(SMW_DEBUG, "%d us\r\n", sum);
	debug_if(SMW_DEBUG, "count=%d\r\n", count);
}

void StarMapWidget::_drawline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const Rect &invalid, colortype color, uint8_t *fb) const
{
	Rect rabs = this->getAbsoluteRect();
	Rect limit = invalid;
	this->translateRectToAbsolute(limit);
	x1 += rabs.x;
	y1 += rabs.y;
	x2 += rabs.x;
	y2 += rabs.y;
	// Transform to FB coordinates
	DisplayTransformation::transformDisplayToFrameBuffer(limit);
	DisplayTransformation::transformDisplayToFrameBuffer(x1, y1);
	DisplayTransformation::transformDisplayToFrameBuffer(x2, y2);

	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, curpixel = 0;

	deltax = abs(x2 - x1); /* The difference between the x's */
	deltay = abs(y2 - y1); /* The difference between the y's */
	x = x1; /* Start x off at the first pixel */
	y = y1; /* Start y off at the first pixel */

	if (x2 >= x1) /* The x-values are increasing */
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else /* The x-values are decreasing */
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) /* The y-values are increasing */
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else /* The y-values are decreasing */
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) /* There is at least one x-value for every y-value */
	{
		xinc1 = 0; /* Don't change the x when numerator >= denominator */
		yinc2 = 0; /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax; /* There are more x-values than y-values */
	}
	else /* There is at least one y-value for every x-value */
	{
		xinc2 = 0; /* Don't change the x for every iteration */
		yinc1 = 0; /* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay; /* There are more y-values than x-values */
	}

//	int lastX = x;
//	int lastY = y;
	int W = HAL::getInstance()->FRAME_BUFFER_WIDTH;
	uint8_t r = 0xFF - Color::getRedColor(color);
	uint8_t g = 0xFF - Color::getGreenColor(color);
	uint8_t b = 0xFF - Color::getBlueColor(color);

	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
//		bool updateLast = false;
		if (limit.intersect(x, y))
		{
			int off = 3 * (y * W + x);
			fb[off] = r;
			fb[off + 1] = g;
			fb[off + 2] = b;
		}
		num += numadd; /* Increase the numerator by the top of the fraction */
		if (num >= den)
		{
			num -= den; /* Calculate the new numerator value */
			x += xinc1; /* Change the x as appropriate */
			y += yinc1; /* Change the y as appropriate */
		}
		x += xinc2; /* Change the x as appropriate */
		y += yinc2; /* Change the y as appropriate */
	}
}
