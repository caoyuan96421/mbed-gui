/*
 * ControllablePWMOut.h
 *
 *  Created on: 2018Äê2ÔÂ9ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_STEPOUT_H_
#define TELESCOPE_STEPOUT_H_

#include "mbed.h"

class StepOut: protected PwmOut
{
public:

	StepOut(PinName pin) :
			PwmOut(pin), stepCount(0), freq(1), status(IDLE)
	{
		this->period(1);
		tim.start();
	}
	virtual ~StepOut()
	{
	}

	void start();
	void stop();

	float setPeriod(float period);
	void resetCount();
	uint64_t getCount();

private:
	typedef enum
	{
		IDLE = 0, STEPPING
	} stepstatus_t;

	uint64_t stepCount;
	float freq;
	stepstatus_t status;
	Timer tim;

};

#endif /* TELESCOPE_STEPOUT_H_ */
