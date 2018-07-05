#include <gui/containers/CoordinatePopup.hpp>
#include <touchgfx/Color.hpp>

CoordinatePopup::CoordinatePopup() :
		callback(NULL), callbackNSClicked(this, &CoordinatePopup::nsClicked), callbackSignClicked(this, &CoordinatePopup::signClicked), callbackok(this, &CoordinatePopup::ok), callbackcancel(this,
				&CoordinatePopup::cancel)
{
	setVisible(false);
	setTouchable(true); // Capture all click event

	unit13.setClickAction(callbackNSClicked);
	unit23.setClickAction(callbackNSClicked);
	sign1.setClickAction(callbackSignClicked);
	sign2.setClickAction(callbackSignClicked);

	okButton.setAction(callbackok);
	cancelButton.setAction(callbackcancel);
	add(ws11);
	add(ws12);
	add(ws13);
	add(ws21);
	add(ws22);
	add(ws23);
}

void CoordinatePopup::show(CoordinateFormat format1, CoordinateFormat format2, const char *name1, const char *name2)
{
	colortype normalTextColor = Color::getColorFrom24BitRGB(90, 25, 65);
	colortype selectedTextColor = Color::getColorFrom24BitRGB(0xA5, 0xA5, 0xA5);
	colortype selectedBackgroundColor = Color::getColorFrom24BitRGB(76, 18, 56);

	ws11.setXY(95, 200);
	switch (format1)
	{
	case FORMAT_D_M_S_SIGNED:
		ws11.setup(72, 100, -24, 25, T_LARGE, 0, 180, 1);
		Unicode::strncpy(sign1Buffer, "+", SIGN1_SIZE);
		Unicode::strncpy(unit11Buffer, "\xB0", UNIT11_SIZE);
		Unicode::strncpy(unit12Buffer, "'", UNIT12_SIZE);
		Unicode::strncpy(unit13Buffer, "\"", UNIT13_SIZE);
		break;
	case FORMAT_H_M_S_SIGNED:
		ws11.setup(72, 100, -24, 25, T_LARGE, 0, 12, 1);
		Unicode::strncpy(sign1Buffer, "+", SIGN1_SIZE);
		Unicode::strncpy(unit11Buffer, "h", UNIT11_SIZE);
		Unicode::strncpy(unit12Buffer, "m", UNIT12_SIZE);
		Unicode::strncpy(unit13Buffer, "s", UNIT13_SIZE);
		break;
	case FORMAT_D_M_S_NS:
		ws11.setup(72, 100, -24, 25, T_LARGE, 0, 90, 1);
		Unicode::strncpy(sign1Buffer, "", SIGN1_SIZE);
		Unicode::strncpy(unit11Buffer, "\xB0", UNIT11_SIZE);
		Unicode::strncpy(unit12Buffer, "'", UNIT12_SIZE);
		Unicode::strncpy(unit13Buffer, "\" N", UNIT13_SIZE);
		break;
	case FORMAT_H_M_S_UNSIGNED:
		ws11.setup(72, 100, -24, 25, T_LARGE, 0, 23, 1);
		Unicode::strncpy(sign1Buffer, "", SIGN1_SIZE);
		Unicode::strncpy(unit11Buffer, "h", UNIT11_SIZE);
		Unicode::strncpy(unit12Buffer, "m", UNIT12_SIZE);
		Unicode::strncpy(unit13Buffer, "s", UNIT13_SIZE);
		break;
	}
	unit11.resizeToCurrentText();
	unit12.resizeToCurrentText();
	unit13.resizeToCurrentText();
	ws11.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 30, 42);

	ws12.setXY(95 + 110, 200);
	ws12.setup(72, 100, -24, 25, T_LARGE, 0, 59, 1);
	ws12.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 30, 42);

	ws13.setXY(95 + 220, 200);
	ws13.setup(72, 100, -24, 25, T_LARGE, 0, 59, 1);
	ws13.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 30, 42);

	ws21.setXY(95, 432);
	switch (format2)
	{
	case FORMAT_D_M_S_SIGNED:
		ws21.setup(72, 100, -24, 25, T_LARGE, 0, 180, 1);
		Unicode::strncpy(sign2Buffer, "+", SIGN2_SIZE);
		Unicode::strncpy(unit21Buffer, "\xB0", UNIT21_SIZE);
		Unicode::strncpy(unit22Buffer, "'", UNIT22_SIZE);
		Unicode::strncpy(unit23Buffer, "\"", UNIT23_SIZE);
		break;
	case FORMAT_H_M_S_SIGNED:
		ws21.setup(72, 100, -24, 25, T_LARGE, 0, 12, 1);
		Unicode::strncpy(sign2Buffer, "+", SIGN2_SIZE);
		Unicode::strncpy(unit21Buffer, "h", UNIT21_SIZE);
		Unicode::strncpy(unit22Buffer, "m", UNIT22_SIZE);
		Unicode::strncpy(unit23Buffer, "s", UNIT23_SIZE);
		break;
	case FORMAT_D_M_S_NS:
		ws21.setup(72, 100, -24, 25, T_LARGE, 0, 90, 1);
		Unicode::strncpy(sign2Buffer, "", SIGN2_SIZE);
		Unicode::strncpy(unit21Buffer, "\xB0", UNIT21_SIZE);
		Unicode::strncpy(unit22Buffer, "'", UNIT22_SIZE);
		Unicode::strncpy(unit23Buffer, "\" N", UNIT23_SIZE);
		break;
	case FORMAT_H_M_S_UNSIGNED:
		ws21.setup(72, 100, -24, 25, T_LARGE, 0, 23, 1);
		Unicode::strncpy(sign2Buffer, "", SIGN2_SIZE);
		Unicode::strncpy(unit21Buffer, "h", UNIT21_SIZE);
		Unicode::strncpy(unit22Buffer, "m", UNIT22_SIZE);
		Unicode::strncpy(unit23Buffer, "s", UNIT23_SIZE);
		break;
	}
	unit21.resizeToCurrentText();
	unit22.resizeToCurrentText();
	unit23.resizeToCurrentText();
	ws21.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 30, 42);

	ws22.setXY(95 + 110, 432);
	ws22.setup(72, 100, -24, 25, T_LARGE, 0, 59, 1);
	ws22.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 30, 42);

	ws23.setXY(95 + 220, 432);
	ws23.setup(72, 100, -24, 25, T_LARGE, 0, 59, 1);
	ws23.setTextColor(normalTextColor, selectedTextColor, selectedBackgroundColor, 30, 42);

	format[0] = format1;
	format[1] = format2;

	Unicode::strncpy(name1Buffer, name1, NAME1_SIZE);
	Unicode::strncpy(name2Buffer, name2, NAME2_SIZE);

	setVisible(true);
	invalidate();
}

void CoordinatePopup::nsClicked(const TextAreaWithOneWildcard& src, const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::PRESSED)
	{
		if (&src == &unit13)
		{
			if (format[0] == FORMAT_D_M_S_NS)
			{
				if ((uint8_t) unit13Buffer[2] == 'N')
					unit13Buffer[2] = 'S';
				else
					unit13Buffer[2] = 'N';
				unit13.invalidate();
			}
		}
		if (&src == &unit23)
		{
			if (format[1] == FORMAT_D_M_S_NS)
			{
				if ((uint8_t) unit23Buffer[2] == 'N')
					unit23Buffer[2] = 'S';
				else
					unit23Buffer[2] = 'N';
				unit23.invalidate();
			}
		}
	}
}

void CoordinatePopup::ok(const AbstractButton&)
{
	double v1, v2;

	switch (format[0])
	{
	case FORMAT_D_M_S_SIGNED:
		v1 = remainder((sign1Buffer[0] == '-' ? -1 : +1) * (ws11.getSelectedValue() + ws12.getSelectedValue() / 60.0 + ws13.getSelectedValue() / 3600.0), 360.0);
		break;
	case FORMAT_H_M_S_SIGNED:
		v1 = remainder((sign1Buffer[0] == '-' ? -1 : +1) * (ws11.getSelectedValue() + ws12.getSelectedValue() / 60.0 + ws13.getSelectedValue() / 3600.0) * 15.0, 360.0);
		break;
	case FORMAT_D_M_S_NS:
		v1 = (ws11.getSelectedValue() + ws12.getSelectedValue() / 60.0 + ws13.getSelectedValue() / 3600.0) * (unit13Buffer[2] == 'N' ? +1 : -1);
		break;
	case FORMAT_H_M_S_UNSIGNED:
		v1 = remainder((ws11.getSelectedValue() + ws12.getSelectedValue() / 60.0 + ws13.getSelectedValue() / 3600.0) * 15.0, 360.0);
		break;
	}

	switch (format[1])
	{
	case FORMAT_D_M_S_SIGNED:
		v2 = remainder((sign2Buffer[0] == '-' ? -1 : +1) * (ws21.getSelectedValue() + ws22.getSelectedValue() / 60.0 + ws23.getSelectedValue() / 3600.0), 360.0);
		break;
	case FORMAT_H_M_S_SIGNED:
		v2 = remainder((sign2Buffer[0] == '-' ? -1 : +1) * (ws21.getSelectedValue() + ws22.getSelectedValue() / 60.0 + ws23.getSelectedValue() / 3600.0) * 15.0, 360.0);
		break;
	case FORMAT_D_M_S_NS:
		v2 = (ws21.getSelectedValue() + ws22.getSelectedValue() / 60.0 + ws23.getSelectedValue() / 3600.0) * (unit23Buffer[2] == 'N' ? +1 : -1);
		break;
	case FORMAT_H_M_S_UNSIGNED:
		v2 = remainder((ws21.getSelectedValue() + ws22.getSelectedValue() / 60.0 + ws23.getSelectedValue() / 3600.0) * 15.0, 360.0);
		break;
	}

	if (callback)
	{
		callback->execute(Coordinate(v1, v2), true);
	}

	setVisible(false);
	invalidate();
	callback = NULL;
}

void CoordinatePopup::signClicked(const TextAreaWithOneWildcard& src, const ClickEvent&evt)
{
	if (evt.getType() == ClickEvent::PRESSED)
	{
		if (&src == &sign1)
		{
			if (format[0] == FORMAT_D_M_S_SIGNED || format[0] == FORMAT_H_M_S_SIGNED)
			{
				if ((uint8_t) sign1Buffer[0] == '+')
					sign1Buffer[0] = '-';
				else
					sign1Buffer[0] = '+';
				sign1.invalidate();
			}
		}
		if (&src == &sign2)
		{
			if (format[1] == FORMAT_D_M_S_SIGNED || format[1] == FORMAT_H_M_S_SIGNED)
			{
				if ((uint8_t) sign2Buffer[0] == '+')
					sign2Buffer[0] = '-';
				else
					sign2Buffer[0] = '+';
				sign2.invalidate();
			}
		}
	}
}

void CoordinatePopup::cancel(const AbstractButton&)
{
	if (callback)
	{
		callback->execute(Coordinate(0, 0), false);
	}
	setVisible(false);
	invalidate();
	callback = NULL;
}
