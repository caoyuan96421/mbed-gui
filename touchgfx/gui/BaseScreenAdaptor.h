/*
 * BaseScreenAdaptor.h
 *
 *  Created on: 2018Äê4ÔÂ29ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_BASESCREENADAPTOR_H_
#define TOUCHGFX_GUI_BASESCREENADAPTOR_H_

#include <gui/include/gui/basescreen_screen/BaseScreenView.hpp>
#include <ctime>

#define MIN_SWIPE_VELOCITY 20

class BaseScreenAdaptor
{
public:
	BaseScreenAdaptor()
	{

	}
	virtual ~BaseScreenAdaptor()
	{
	}

//	virtual void setTime(time_t timestamp)
//	{
//		baseview.setTime(timestamp);
//	}
//
//	virtual void setEqCoords(const EquatorialCoordinates &eq)
//	{
//		baseview.setEqCoords(eq);
//	}

	BaseScreenView baseview;
};

#endif /* TOUCHGFX_GUI_BASESCREENADAPTOR_H_ */
