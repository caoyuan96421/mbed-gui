#include <gui/basescreen_screen/BaseScreenView.hpp>

BaseScreenView::BaseScreenView()
{

}

void BaseScreenView::setupScreen()
{

}

void BaseScreenView::tearDownScreen()
{

}

void BaseScreenView::addTo(Container *c)
{
	if (!c)
		return;
	while (true)
	{
		Drawable *d = container.getFirstChild();
		if (!d)
			break;
		container.remove(*d);
		c->add(*d);
	}
}

void BaseScreenView::setTime(time_t timestamp, int tz)
{

	timestamp += tz * 3600;

	struct tm ts;
	gmtime_r(&timestamp, &ts);

	char buf[32];
	Unicode::UnicodeChar bufU[32];
	// Print time
	strftime(buf, sizeof(buf), "%X", &ts);
	Unicode::strncpy(bufU, buf, 32);
	Unicode::snprintf(timeBuffer, TIME_SIZE, "UTC%+d %s", tz, bufU);
	time.invalidate();

	// Print date
	strftime(buf, sizeof(buf), "%Y %b %d %a", &ts);
	Unicode::strncpy(dateBuffer, buf, DATE_SIZE);
	date.invalidate();
}

void BaseScreenView::setEqCoords(const EquatorialCoordinates& eq)
{
	char we = (eq.ra > 0) ? 'E' : 'W';
	char ns = (eq.dec > 0) ? 'N' : 'S';
	double r = (eq.ra < 0) ? eq.ra + 360.0 : eq.ra;
	double d = fabs(eq.dec);

	char buf[32];

	// RA string
	snprintf(buf, sizeof(buf), "%2dh%02d'%02d\"%c", int(r / 15), int(fmod(r, 15.0) * 4), (int) floor(fmod(r, 0.25) * 240), we);
	Unicode::strncpy(ra_coordBuffer, buf, RA_COORD_SIZE);
	ra_coord.invalidate();

	// DEC string
	snprintf(buf, sizeof(buf), "%2d\x00b0%02d'%02d\"%c", int(d), int(fmod(d, 1.0) * 60), (int) floor(fmod(d, 1.0 / 60) * 3600), ns);
	Unicode::strncpy(dec_coordBuffer, buf, DEC_COORD_SIZE);
	dec_coord.invalidate();

}
