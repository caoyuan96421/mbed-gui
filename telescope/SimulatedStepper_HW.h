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
#include <StepOut.h>

class SimulatedStepper_HW: public StepperMotor
{
public:
	SimulatedStepper_HW(PinName in, PinName out) :
			stepout(out), irqin(in), stepCount(0), stepCount_HW(0), period(1), status(
					IDLE), inc(1)
	{
		irqin.fall(callback(cb, this));
	}
	virtual ~SimulatedStepper_HW()
	{
	}

	void start(stepdir_t dir)
	{
		if (status == IDLE)
		{
			core_util_critical_section_enter();
			status = STEPPING;
			inc = (dir == StepperMotor::STEP_FORWARD) ? 1 : -1;
			stepout.start();
			stepout.resetCount();
			core_util_critical_section_exit();
		}
	}
	void stop()
	{
		if (status == STEPPING)
		{
			core_util_critical_section_enter();
			status = IDLE;
			stepout.stop();
			stepCount += stepout.getCount() * inc;

			core_util_critical_section_exit();
		}
	}

	/**
	 * Wait for N steps to be stepped and return
	 */
	void stepN(stepdir_t dir, uint64_t N)
	{
	}

	float setPeriod(float period)
	{
		if (period > 0)
		{
			if (period > 2e-5)
				if (status == IDLE)
				{
					this->period = stepout.setPeriod(period);
				}
				else
				{
					core_util_critical_section_enter();
					stop();
					this->period = stepout.setPeriod(period);
					start(
							inc == 1 ?
									StepperMotor::STEP_FORWARD :
									StepperMotor::STEP_BACKWARD);
					core_util_critical_section_exit();
				}
			else
			{
				error("Period too small: %f", period);
			}
			return this->period;
		}
		else
			return 0;
	}

	int64_t getStepCount()
	{
		if (status == IDLE)
			return stepCount;
		else
			return stepCount + stepout.getCount() * inc;
	}

	int64_t getStepCountHW()
	{
		return stepCount_HW;
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

	StepOut stepout;
	InterruptIn irqin;

	int64_t stepCount;
	int64_t stepCount_HW;
	float period;
	stepstatus_t status;
	int32_t inc;

	static void cb(SimulatedStepper_HW *stepper)
	{
		stepper->stepCount_HW += stepper->inc;
	}
};

#endif /* TELESCOPE_SIMULATEDSTEPPER_H_ */
