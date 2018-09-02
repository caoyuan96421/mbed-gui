#include <gui/polaralignscreen_screen/PolarAlignScreenView.hpp>
#include <touchgfx/Color.hpp>

PolarAlignScreenView::PolarAlignScreenView() :
		buttonCallback(this, &PolarAlignScreenView::buttonPressed), starSelectedCallback(this, &PolarAlignScreenView::starSelected)
{
	baseview.addTo(&container);
	textArea2.setWideTextAction(WIDE_TEXT_WORDWRAP_ELLIPSIS_AFTER_SPACE);
	textArea5.setWideTextAction(WIDE_TEXT_WORDWRAP_ELLIPSIS_AFTER_SPACE);
	textArea5.setVisible(false);
	pa_state = RESET;

	defaultColor = Color::getColorFrom24BitRGB(25, 25, 25);
	selectedColor = Color::getColorFrom24BitRGB(125, 125, 125);
	pressedColor = Color::getColorFrom24BitRGB(45, 45, 75);

	time_now = time(NULL);
	minAlt = 5;
	height = 40;
	num_stars = 0;
	selectedStar = NULL;

	container_stars.setPosition(0, 0, scrollableContainer1.getWidth(), 0);
	scrollableContainer1.add(container_stars);

	calib = NULL;

	buttonContinue.setAction(buttonCallback);
	buttonAligned.setAction(buttonCallback);
	buttonRestart.setAction(buttonCallback);
	buttonStop.setAction(buttonCallback);
}

PolarAlignScreenView::~PolarAlignScreenView()
{
	if (calib)
	{
		delete calib;
	}
}

void PolarAlignScreenView::setupScreen()
{
	listStars();
}

static void clearMenu(Container &c)
{
	ButtonItem *d = (ButtonItem *) c.getFirstChild();
	while (d)
	{
		ButtonItem *q = d;
		d = (ButtonItem *) d->getNextSibling();
		c.remove(*q);
		if (q->getName())
			delete[] q->getName();
		if (q->getValue())
			delete[] q->getValue();
		delete q;
	}
}

void PolarAlignScreenView::tearDownScreen()
{
	TelescopeBackend::updateAlignment();
}

void PolarAlignScreenView::callback(StarInfo* s, void* arg)
{
	if (arg)
		((PolarAlignScreenView *) arg)->_callback(s);
}

void PolarAlignScreenView::_callback(StarInfo* star)
{
	EquatorialCoordinates eq(star->DEC, star->RA);

	LocalEquatorialCoordinates leq = CelestialMath::equatorialToLocalEquatorial(eq, time_now, loc);
	AzimuthalCoordinates az = CelestialMath::localEquatorialToAzimuthal(leq, loc);

	if (az.alt >= minAlt)
	{
		ButtonItem &item = *new ButtonItem();

		item.setWidth(container_stars.getWidth());
		item.setHeight(height);
		item.setXY(0, height * num_stars);

		const int max_len = 32;
		Unicode::UnicodeChar *name = new Unicode::UnicodeChar[max_len];
		Unicode::strncpy(name, star->name, max_len);
		item.setName(name);
		item.setReleasedColor(defaultColor);
		item.setPressedColor(pressedColor);
		item.setUserData(star);
		item.setAction(starSelectedCallback);

		container_stars.add(item);
		num_stars++;
	}
}

void PolarAlignScreenView::starSelected(const AbstractButton& src)
{
	if (const_cast<AbstractButton &>(src).getParent() == &container_stars)
	{
		char buf[256];
		ButtonItem &button = const_cast<ButtonItem &>((const ButtonItem &) src);
		if (selectedStar)
		{
			selectedStar->setReleasedColor(defaultColor);
		}
		selectedStar = &button;
		button.setReleasedColor(selectedColor);

		StarInfo *star = (StarInfo *) button.getUserData();

		EquatorialCoordinates eq(star->DEC, star->RA);

		LocalEquatorialCoordinates leq = CelestialMath::equatorialToLocalEquatorial(eq, time_now, loc);
		AzimuthalCoordinates az = CelestialMath::localEquatorialToAzimuthal(leq, loc);
		double r = (eq.ra < 0) ? eq.ra + 360.0 : eq.ra;
		double d = fabs(eq.dec);
		double h = (leq.ha < 0) ? leq.ha + 360.0 : leq.ha;

		snprintf(buf, sizeof(buf), "RA: %2dh%02d'%02d\"\n"
				"HA: %2dh%02d'%02d\"\n"
				"Dec: %c%2d\x00b0%02d'%02d\"\n"
				"Altitude: %.2f\x00b0\n"
				"Magnitude: %.2f\n", int(r / 15), int(fmod(r, 15.0) * 4), (int) round(fmod(r, 0.25) * 240), int(h / 15), int(fmod(h, 15.0) * 4), (int) round(fmod(h, 0.25) * 240),
				eq.dec > 0 ? '+' : '-', int(d), int(fmod(d, 1.0) * 60), (int) round(fmod(d, 1.0 / 60) * 3600), az.alt, star->magnitude);

		Unicode::strncpy(text_descriptionBuffer, buf, TEXT_DESCRIPTION_SIZE);
		text_description.invalidate();
	}
}

void PolarAlignScreenView::listStars()
{
	pa_state = STEP1_CHOOSESTAR;
	textArea2.setVisible(false);
	textArea3.setVisible(false);
	textArea4.setVisible(false);
	buttonAligned.setVisible(false);

	TelescopeBackend::updateAlignment();
	time_now = time(NULL);
	loc = LocationCoordinates(TelescopeBackend::getConfigDouble("latitude"), TelescopeBackend::getConfigDouble("longitude"));
	num_stars = 0;
	selectedStar = NULL;

	clearMenu(container_stars);
	StarCatalog::getInstance().query_common(&callback, -180, 180, -90, 90, this, 100);

	container_stars.setHeight(height * num_stars);
	scrollableContainer1.childGeometryChanged();

	getRootContainer().invalidate();
}

void PolarAlignScreenView::slewToSelected()
{
	pa_state = STEP2_SLEWTOTARGET;
	textArea2.setVisible(true);
	textArea3.setVisible(true);
	textArea4.setVisible(true);
	buttonAligned.setVisible(true);

	// Get previous calibration
	if (!calib)
	{
		calib = new EqCalibration();
	}
	TelescopeBackend::getCalibration(*calib);

	// Check the error is not crazy
	if (calib->error > 1.0)
	{
		textArea5.setVisible(true);
		textArea5.invalidate();
		pa_state = ERROR;
		return;
	}

	EqCalibration newcalib = *calib;
	// Set PA error to zero
	newcalib.pa.alt = loc.lat;
	newcalib.pa.azi = 0;

	// Set calibration to mount
	TelescopeBackend::setCalibration(newcalib);

	// Slew to the target position
	if (selectedStar && selectedStar->getUserData())
	{
		StarInfo *star = (StarInfo *) selectedStar->getUserData();
		TelescopeBackend::goTo(EquatorialCoordinates(star->DEC, star->RA));
	}

	getRootContainer().invalidate();
}

void PolarAlignScreenView::completePA()
{
	pa_state = STEP3_COMPLETEPA;

	TelescopeBackend::forceAlignment();
	TelescopeBackend::updateAlignment();
}

void PolarAlignScreenView::buttonPressed(const AbstractButton& src)
{
	if (&src == &this->buttonRestart && pa_state != ERROR)
	{
		listStars();
	}
	else if (&src == &this->buttonContinue && pa_state == STEP1_CHOOSESTAR && selectedStar != NULL)
	{
		slewToSelected();
	}
	else if (&src == &this->buttonAligned && pa_state == STEP2_SLEWTOTARGET && selectedStar != NULL)
	{
		completePA();
	}
	else if(&src == &this->buttonStop){
		TelescopeBackend::emergencyStop();
	}
}
