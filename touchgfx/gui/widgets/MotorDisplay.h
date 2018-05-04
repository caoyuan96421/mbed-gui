/*
 * MotorDisplay.h
 *
 *  Created on: 2018Äê2ÔÂ13ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_WIDGETS_MOTORDISPLAY_H_
#define TOUCHGFX_GUI_WIDGETS_MOTORDISPLAY_H_

#include <touchgfx/containers/Container.hpp>
#include <touchgfx/widgets/canvas/Circle.hpp>
#include <touchgfx/widgets/canvas/Shape.hpp>
#include <touchgfx/widgets/canvas/PainterRGB888.hpp>

using namespace touchgfx;

class MotorDisplay: public Container
{
protected:
	class Needle: public Shape<3>
	{
	public:
		Needle()
		{
			ShapePoint<float> points[3] =
			{
			{ -2.5f, 0 },
			{ 0.0f, -100.0f },
			{ 2.5f, 0 } };
			setShape(points);
		}
	};
public:
	MotorDisplay();
	virtual ~MotorDisplay()
	{
	}

	float angle;
	Circle bounds;
	Needle finger;
	PainterRGB888 painter;
	PainterRGB888 needlepainter;

	void setAngle(float ang);

	float getAngle() const
	{
		return angle;
	}
};

#endif /* TOUCHGFX_GUI_WIDGETS_MOTORDISPLAY_H_ */
