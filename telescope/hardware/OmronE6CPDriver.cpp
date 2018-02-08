/*
 * OmronE6CPDriver.cpp
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#include <telescope/hardware/OmronE6CPDriver.h>
#include "mbed.h"

OmronE6CPDriver::OmronE6CPDriver(DigitalOut *c, DigitalOut *l, DigitalIn *d) :
		GrayAbsEncoder(8), clk(c), load(l), data(d) {
	if (clk == NULL || load == NULL || data == NULL) {
		fprintf(stderr, "ERROR in OmronE6CPDriver: PINs undefined");
		return;
	}
	/*Initialize pin states*/
	*clk = 0;
	*load = 1;
}

uint32_t OmronE6CPDriver::readPosGray() {
	if (clk == NULL || load == NULL || data == NULL) {
		fprintf(stderr, "ERROR in OmronE6CPDriver: PINs undefined");
		return 0;
	}
	uint32_t value = 0;
	/*Load the shift register*/
	*clk = 0;
	*load = 0;

//	delay();

	/*Lock the shift register*/
	*load = 1;
	for (int i = 0; i < bitCount; i++) {
		/*Read value*/
		value = (value << 1) + (1 ^ (uint32_t) (*data)); // Negated because of the open-drain driving output of the encoder
		/*Clock. Now the next bit will be loaded*/
		*clk = 1;
		/*Delay*/
		//delay();
		/*Reset clock*/
		*clk = 0;
		/*Delay*/
		//delay();
	}
	return value;
}
