/*
 * SimulatedStepper.h
 *
 *  Created on: 2018Äê2ÔÂ11ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_SIMULATEDSTEPPER_H_
#define TELESCOPE_SIMULATEDSTEPPER_H_

#include "mbed.h"
#include <StepperMotor.h>

class SimulatedStepper: public StepperMotor
{
public:
	SimulatedStepper() :
			stepCount(0), period(1), status(IDLE), inc(1)
	{
		tick.detach();
	}
	virtual ~SimulatedStepper()
	{
	}

	void start(stepdir_t dir)
	{
		if (status == IDLE)
		{
			core_util_critical_section_enter();
			status = STEPPING;
			inc = (dir == StepperMotor::STEP_FORWARD) ? 1 : -1;
			tick.attach(callback(this->cb, this), period);
			core_util_critical_section_exit();
		}
	}
	void stop()
	{
		if (status == STEPPING)
		{
			core_util_critical_section_enter();
			status = IDLE;
			tick.detach();
			core_util_critical_section_exit();
		}
	}

	/**
	 * Wait for N steps to be stepped and return
	 */
	void stepN(stepdir_t dir, uint64_t N)
	{
	}

	void setPeriod(float period)
	{
		if (period > 0)
		{
			if (period > 2e-5)
				if (status == IDLE)
					this->period = period;
				else
				{
					core_util_critical_section_enter();
					tick.detach();
					this->period = period;
					tick.attach(callback(this->cb, this), period);
					core_util_critical_section_exit();
				}
			else
			{
				error("Period too small: %f", period);
			}
		}
	}

	int64_t getStepCount()
	{
		return stepCount;
	}

	void setStepCount(int64_t set)
	{
		stepCount = set;
	}
private:
	Ticker tick;
	typedef enum
	{
		IDLE = 0, STEPPING
	} stepstatus_t;

	int64_t stepCount;
	float period;
	stepstatus_t status;
	int32_t inc;

	static void cb(SimulatedStepper *stepper)
	{
		stepper->stepCount += stepper->inc;
	}
};

#endif /* TELESCOPE_SIMULATEDSTEPPER_H_ */
