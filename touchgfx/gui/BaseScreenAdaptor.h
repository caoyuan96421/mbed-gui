/*
 * BaseScreenAdaptor.h
 *
 *  Created on: 2018��4��29��
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_BASESCREENADAPTOR_H_
#define TOUCHGFX_GUI_BASESCREENADAPTOR_H_

#include <gui/include/gui/basescreen_screen/BaseScreenView.hpp>
#include <ctime>

class BaseScreenAdaptor
{
public:
	BaseScreenAdaptor()
	{

	}
	virtual ~BaseScreenAdaptor()
	{
	}

	void setTime(time_t timestamp)
	{
		baseview.setTime(timestamp);
	}

	void setEqCoords(const EquatorialCoordinates &eq)
	{
		baseview.setEqCoords(eq);
	}

	BaseScreenView baseview;
};

#endif /* TOUCHGFX_GUI_BASESCREENADAPTOR_H_ */
