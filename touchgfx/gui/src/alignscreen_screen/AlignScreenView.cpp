#include <gui/alignscreen_screen/AlignScreenView.hpp>
#include <touchgfx/Color.hpp>

AlignScreenView::AlignScreenView() :
		buttonRefreshCallback(this, &AlignScreenView::buttonRefreshPressed), buttonAddCallback(this, &AlignScreenView::buttonAddPressed), buttonDeleteCallback(this,
				&AlignScreenView::buttonDeletePressed), buttonGotoCallback(this, &AlignScreenView::buttonGotoPressed), buttonAlignCallback(this, &AlignScreenView::buttonAlignPressed), starSelectedCallback(
				this, &AlignScreenView::starSelected)
{
	baseview.addTo(&container);

	buttonRefresh.setAction(buttonRefreshCallback);
	buttonAdd.setAction(buttonAddCallback);
	buttonDelete.setAction(buttonDeleteCallback);
	buttonGoto.setAction(buttonGotoCallback);
	buttonAlign.setAction(buttonAlignCallback);

	time_now = time(NULL);

	minAlt = 5;
	height = 40;
	num_candidates = 0;
	num_alignment = 0;

	defaultColor = Color::getColorFrom24BitRGB(25, 25, 25);
	selectedColor = Color::getColorFrom24BitRGB(125, 125, 125);
	pressedColor = Color::getColorFrom24BitRGB(45, 45, 75);

	selectedCandidate = NULL;
	selectedAlignment = NULL;

	container_candidates.setPosition(0, 0, scrollableContainer1.getWidth(), 0);
	container_selected.setPosition(0, 0, scrollableContainer1.getWidth(), 0);

	scrollableContainer1.add(container_candidates);
	scrollableContainer2.add(container_selected);
}

void AlignScreenView::setupScreen()
{
	updateMenu();
}

void AlignScreenView::tearDownScreen()
{
	clearMenu(container_candidates);
	clearMenu(container_selected);
}

void AlignScreenView::updateMenu()
{
	time_now = time(NULL);
	loc = LocationCoordinates(TelescopeBackend::getConfigDouble("latitude"), TelescopeBackend::getConfigDouble("longitude"));
	num_candidates = 0;
	selectedCandidate = NULL;

	clearMenu(container_candidates);
	StarCatalog::getInstance().query_common(&callback, -180, 180, -90, 90, this, 100);

	container_candidates.setHeight(height * num_candidates);
	scrollableContainer1.childGeometryChanged();

	clearMenu(container_selected);

	// Add existing alignment star
	num_alignment = TelescopeBackend::getNumAlignmentStars();

	AlignmentStar as;
	for (int i = 0; i < num_alignment; i++)
	{
		const char *name = "Undefined star";
		TelescopeBackend::getAlignmentStar(i, as);

		StarInfo *star = StarCatalog::getInstance().searchByCoordinates(as.star_ref.ra, as.star_ref.dec, 1e-3);
		if (star)
		{
			name = star->name;
		}

		ButtonItem &item = *new ButtonItem();

		item.setWidth(container_selected.getWidth());
		item.setHeight(height);
		item.setXY(0, height * i);

		const int max_len = 32;
		Unicode::UnicodeChar *uname = new Unicode::UnicodeChar[max_len];
		Unicode::strncpy(uname, name, max_len);
		item.setName(uname);
		if (i == 0)
		{
			item.setReleasedColor(selectedColor);
			selectedAlignment = &item;
		}
		else
			item.setReleasedColor(defaultColor);
		item.setPressedColor(pressedColor);
		item.setUserData(star);
		item.setAction(starSelectedCallback);

		container_selected.add(item);
	}

	container_selected.setHeight(height * num_alignment);
	scrollableContainer2.childGeometryChanged();

	updateCalibration();

	getRootContainer().invalidate();
}

void AlignScreenView::callback(StarInfo* s, void* arg)
{
	if (arg)
		((AlignScreenView *) arg)->_callback(s);
}

void AlignScreenView::_callback(StarInfo* star)
{

	EquatorialCoordinates eq(star->DEC, star->RA);

	LocalEquatorialCoordinates leq = CelestialMath::equatorialToLocalEquatorial(eq, time_now, loc);
	AzimuthalCoordinates az = CelestialMath::localEquatorialToAzimuthal(leq, loc);

	if (az.alt >= minAlt)
	{
		ButtonItem &item = *new ButtonItem();

		item.setWidth(container_candidates.getWidth());
		item.setHeight(height);
		item.setXY(0, height * num_candidates);

		const int max_len = 32;
		Unicode::UnicodeChar *name = new Unicode::UnicodeChar[max_len];
		Unicode::strncpy(name, star->name, max_len);
		item.setName(name);
		item.setReleasedColor(defaultColor);
		item.setPressedColor(pressedColor);
		item.setUserData(star);
		item.setAction(starSelectedCallback);

		container_candidates.add(item);
		num_candidates++;
	}
}

void AlignScreenView::starSelected(const AbstractButton& src)
{
	if (const_cast<AbstractButton &>(src).getParent() == &container_candidates)
	{
		char buf[256];
		ButtonItem &button = const_cast<ButtonItem &>((const ButtonItem &) src);
		if (selectedCandidate)
		{
			selectedCandidate->setReleasedColor(defaultColor);
		}
		selectedCandidate = &button;
		button.setReleasedColor(selectedColor);

		StarInfo *star = (StarInfo *) button.getUserData();

		EquatorialCoordinates eq(star->DEC, star->RA);

		LocalEquatorialCoordinates leq = CelestialMath::equatorialToLocalEquatorial(eq, time_now, loc);
		AzimuthalCoordinates az = CelestialMath::localEquatorialToAzimuthal(leq, loc);
		double r = (eq.ra < 0) ? eq.ra + 360.0 : eq.ra;
		double d = fabs(eq.dec);
		double h = (leq.ha < 0) ? leq.ha + 360.0 : leq.ha;

		snprintf(buf, sizeof(buf), "RA: %2dh%02d'%02d\"\n"
				"Dec: %c%2d\x00b0%02d'%02d\"\n"
				"HA: %2dh%02d'%02d\"\n"
				"Altitude: %.2f\x00b0\n"
				"Magnitude: %.2f\n", int(r / 15), int(fmod(r, 15.0) * 4), (int) round(fmod(r, 0.25) * 240), eq.dec > 0 ? '+' : '-', int(d), int(fmod(d, 1.0) * 60),
				(int) round(fmod(d, 1.0 / 60) * 3600), int(h / 15), int(fmod(h, 15.0) * 4), (int) round(fmod(h, 0.25) * 240), az.alt, star->magnitude);

		Unicode::strncpy(text_descriptionBuffer, buf, TEXT_DESCRIPTION_SIZE);
		text_description.invalidate();
	}
	else if (const_cast<AbstractButton &>(src).getParent() == &container_selected)
	{
		ButtonItem &button = const_cast<ButtonItem &>((const ButtonItem &) src);
		if (selectedAlignment)
		{
			selectedAlignment->setReleasedColor(defaultColor);
		}
		selectedAlignment = &button;
		button.setReleasedColor(selectedColor);

		container_selected.invalidate();
	}
}

void AlignScreenView::buttonAddPressed(const AbstractButton&)
{
	if (selectedCandidate != NULL)
	{
		StarInfo *star = (StarInfo *) selectedCandidate->getUserData();
		if (!star)
			return;

		// Check if this star is already added
		bool alreadySelected = false;
		ButtonItem *b = (ButtonItem *) container_selected.getFirstChild();
		while (b)
		{
			if (((StarInfo *) b->getUserData()) == star)
			{
				alreadySelected = true;
				break;
			}
			b = (ButtonItem *) b->getNextSibling();
		}
		if (alreadySelected)
		{
			return;
		}

		// Add alignment star, but keep current calibration
		AlignmentStar as;
		as.star_ref = EquatorialCoordinates(star->DEC, star->RA);
		as.star_meas = TelescopeBackend::convertEquatorialToMount(as.star_ref);
		TelescopeBackend::addAlignmentStar(as);

		updateCalibration();

		ButtonItem &item = *new ButtonItem();

		item.setWidth(container_selected.getWidth());
		item.setHeight(height);
		item.setXY(0, height * num_alignment);

		const int max_len = 32;
		Unicode::UnicodeChar *name = new Unicode::UnicodeChar[max_len];
		Unicode::strncpy(name, star->name, max_len);
		item.setName(name);
		item.setReleasedColor(selectedColor);
		item.setPressedColor(pressedColor);
		item.setUserData(star);
		item.setAction(starSelectedCallback);

		container_selected.add(item);
		num_alignment++;
		container_selected.setHeight(height * num_alignment);

		if (selectedAlignment)
		{
			selectedAlignment->setReleasedColor(defaultColor);
		}
		selectedAlignment = &item;

		container_selected.invalidate();
		scrollableContainer2.childGeometryChanged();
	}
}

void AlignScreenView::buttonDeletePressed(const AbstractButton&)
{
	if (selectedAlignment != NULL && selectedAlignment->getUserData() != NULL)
	{
		ButtonItem *d = (ButtonItem *) selectedAlignment->getNextSibling();
		StarInfo *si = (StarInfo*) selectedAlignment->getUserData();

		while (d)
		{
			d->setY(d->getY() - height);
			d = (ButtonItem *) d->getNextSibling();
		}

		container_selected.remove(*selectedAlignment);

		for (int i = 0; i < num_alignment; i++)
		{
			AlignmentStar star;
			TelescopeBackend::getAlignmentStar(i, star);
//			printf("star %d, %f, %f, %f, %f\r\n", i, star.star_ref.ra, star.star_ref.dec, star.star_meas.ra_delta, star.star_meas.dec_delta);
			if (fabs(star.star_ref.ra - si->RA) < 1e-3 && fabs(star.star_ref.dec - si->DEC) < 1e-3)
			{
				TelescopeBackend::removeAlignmentStar(i);
				break;
			}
		}

		updateCalibration();

		if (selectedAlignment->getName())
			delete[] selectedAlignment->getName();
		if (selectedAlignment->getValue())
			delete[] selectedAlignment->getValue();
		delete selectedAlignment;

		selectedAlignment = NULL;
		num_alignment--;
		container_selected.invalidate();
		container_selected.setHeight(height * num_alignment);
		container_selected.invalidate();
		scrollableContainer2.childGeometryChanged();
	}
}

void AlignScreenView::buttonGotoPressed(const AbstractButton&)
{
	if (selectedAlignment != NULL && selectedAlignment->getUserData() != NULL)
	{
		StarInfo *star = (StarInfo*) selectedAlignment->getUserData();
		TelescopeBackend::goTo(EquatorialCoordinates(star->DEC, star->RA));
	}
}

void AlignScreenView::buttonAlignPressed(const AbstractButton&)
{
	if (selectedAlignment != NULL && selectedAlignment->getUserData() != NULL)
	{
		StarInfo *si = (StarInfo*) selectedAlignment->getUserData();
		int index = -1;
		for (int i = 0; i < num_alignment; i++)
		{
			AlignmentStar star;
			TelescopeBackend::getAlignmentStar(i, star);
			if (fabs(star.star_ref.ra - si->RA) < 1e-3 && fabs(star.star_ref.dec - si->DEC) < 1e-3)
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			TelescopeBackend::replaceAlignmentStar(index, EquatorialCoordinates(si->DEC, si->RA));
			updateCalibration();
		}
	}
}

void AlignScreenView::updateCalibration()
{
	char buf[256];

	EqCalibration calib;
	TelescopeBackend::getCalibration(calib); // Obtain the calibration only
	snprintf(buf, sizeof(buf), "RA Index: %.3f\x00b0\n"
			"Dec Index: %.3f\x00b0\n"
			"Cone: %.3f\x00b0\n"
			"PAE Alt: %.3f\x00b0\n"
			"PAE Az: %.3f\x00b0\n"
			"Error: %.3f\x00b0\n", calib.offset.ra_off, calib.offset.dec_off, calib.cone, calib.pa.alt - TelescopeBackend::getConfigDouble("latitude"), calib.pa.azi, calib.error);

	Unicode::strncpy(text_alignmentBuffer, buf, TEXT_ALIGNMENT_SIZE);
	text_alignment.invalidate();
}

void AlignScreenView::clearMenu(Container &c)
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
