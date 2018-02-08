/*
 * OmronE6CPDriver.h
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_HARDWARE_OMRONE6CPDRIVER_H_
#define TELESCOPE_HARDWARE_OMRONE6CPDRIVER_H_

#include <telescope/GrayAbsEncoder.h>
#include "mbed.h"

class OmronE6CPDriver: public GrayAbsEncoder {
public:
	OmronE6CPDriver(DigitalOut *c, DigitalOut *l, DigitalIn *d);

private:
	DigitalOut *clk;
	DigitalOut *load;
	DigitalIn *data;

	uint32_t readPosGray();
};

#endif /* TELESCOPE_HARDWARE_OMRONE6CPDRIVER_H_ */
