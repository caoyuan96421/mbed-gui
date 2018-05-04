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

void BaseScreenView::setTime(time_t timestamp)
{
	struct tm ts;
	gmtime_r(&timestamp, &ts);

	char buf[32];
	// Print time
	strftime(buf, sizeof(buf), "%X", &ts);
	Unicode::strncpy(timeBuffer, buf, TIME_SIZE);
	time.invalidate();

	// Print date
	strftime(buf, sizeof(buf), "%Y %b %d %a", &ts);
	Unicode::strncpy(dateBuffer, buf, DATE_SIZE);
	date.invalidate();
}
