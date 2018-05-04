/*
 * MotorDisplay.cpp
 *
 *  Created on: 2018Äê2ÔÂ13ÈÕ
 *      Author: caoyuan9642
 */

#include <touchgfx/gui/widgets/MotorDisplay.h>
#include <touchgfx/Color.hpp>

MotorDisplay::MotorDisplay() :
		angle(0.0f)
{
	setTouchable(false);

	painter.setColor(Color::getColorFrom24BitRGB(0xFF, 0x67, 0x23), 0x8F);
	needlepainter.setColor(Color::getColorFrom24BitRGB(0x00, 0x7F, 0xDF), 0xFF);

	bounds.setPosition(0, 0, 200, 200);
	bounds.setCenter(100, 100);
	bounds.setRadius(95);
	bounds.setLineWidth(5);
	bounds.setPainter(painter);
	add(bounds);

	finger.setOrigin(100, 100);
	finger.setPainter(needlepainter);
	finger.setPosition(0, 0, 200, 200);
	add(finger);

	setWidth(200);
	setHeight(200);
}

void MotorDisplay::setAngle(float ang)
{
	finger.updateAngle((int) ang);
}
