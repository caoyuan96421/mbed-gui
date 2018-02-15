/*
 * GenericStepperMotor.h
 *
 *  Created on: 2018Äê2ÔÂ8ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_STEPPERMOTOR_H_
#define TELESCOPE_STEPPERMOTOR_H_

#include <stdint.h>

/**
 * Interface of a generic stepper motor
 */
class StepperMotor
{
public:
	typedef enum
	{
		STEP_FORWARD = 0, STEP_BACKWARD = 1
	} stepdir_t;

public:
	StepperMotor()
	{
	}
	virtual ~StepperMotor()
	{
	}

	virtual void start(stepdir_t dir) = 0;
	virtual void stop() = 0;

	/**
	 * Wait for N steps to be stepped and return
	 */
	virtual void stepN(stepdir_t dir, uint64_t N) = 0;

	/**
	 * Set stepping period. Returns the actual period set (the error may be significant to affect GoTo accuracy
	 */
	virtual float setPeriod(float period) = 0;

	virtual int64_t getStepCount() const = 0;

	virtual void setStepCount(int64_t) = 0;

	/*Set frequency of the stepping*/
	float setFrequency(float freq)
	{
		if (freq > 0)
			return 1.0f/setPeriod(1.0f / freq);
		else
			return 0;
	}

	/*Optional: current settings*/

	virtual void setCurrent(float current)
	{
	}

};

inline StepperMotor::stepdir_t operator!(const StepperMotor::stepdir_t dir)
{
	if (dir == StepperMotor::STEP_FORWARD)
		return StepperMotor::STEP_BACKWARD;
	else
		return StepperMotor::STEP_FORWARD;
}

#endif /* TELESCOPE_STEPPERMOTOR_H_ */
