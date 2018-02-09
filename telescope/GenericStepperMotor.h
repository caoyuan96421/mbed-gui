/*
 * GenericStepperMotor.h
 *
 *  Created on: 2018Äê2ÔÂ8ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_GENERICSTEPPERMOTOR_H_
#define TELESCOPE_GENERICSTEPPERMOTOR_H_

#include <stdint.h>

class GenericStepperMotor {
public:
	typedef enum {
		STEP_FORWARD = 0, STEP_BACKWARD
	} stepdir_t;

private:
	stepdir_t stepDirection;

public:
	GenericStepperMotor();
	virtual ~GenericStepperMotor();

	/**
	 * Generate one plus for one microstep
	 */
	virtual void step() {
	}

	virtual void setStepDirection(stepdir_t stepDirection) {
		this->stepDirection = stepDirection;
	}

	stepdir_t getStepDirection() const {
		return stepDirection;
	}
};
inline GenericStepperMotor::stepdir_t operator!(
		const GenericStepperMotor::stepdir_t dir) {
	if (dir == GenericStepperMotor::STEP_FORWARD)
		return GenericStepperMotor::STEP_BACKWARD;
	else
		return GenericStepperMotor::STEP_FORWARD;
}

#endif /* TELESCOPE_GENERICSTEPPERMOTOR_H_ */
