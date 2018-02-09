/*
 * AMIS30543StepperDriver.h
 *
 *  Created on: 2018Äê2ÔÂ8ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_DRIVER_AMIS30543STEPPERDRIVER_H_
#define TELESCOPE_DRIVER_AMIS30543STEPPERDRIVER_H_

#include <GenericStepperMotor.h>
#include "mbed.h"

class AMIS30543StepperDriver: public GenericStepperMotor, SPI {
public:
	AMIS30543StepperDriver(PinName mosi, PinName miso, PinName sclk, PinName cs,
			PinName step, PinName dir = NC, PinName err = NC);
	virtual ~AMIS30543StepperDriver();

	void step();
	void setStepDirection(stepdir_t);

private:
	DigitalOut nextstep;
	DigitalOut cs;
	DigitalOut dir;
	DigitalIn err;

	bool useCS, useDIR, useERR;

	void assertCS();
	void deassertCS();
};

#endif /* TELESCOPE_DRIVER_AMIS30543STEPPERDRIVER_H_ */
