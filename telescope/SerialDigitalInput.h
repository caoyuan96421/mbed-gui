/*
 * SerialDigitalInput.h
 *
 *  Created on: 2018Äê2ÔÂ10ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_SERIALDIGITALINPUT_H_
#define TELESCOPE_SERIALDIGITALINPUT_H_

#include "mbed.h"

/**
 * A serial interfaced digital input
 * Example application is to read a parallel input (8~32 bits) using cascaded 74xx165.
 */
template<uint8_t N>
class SerialDigitalInput
{
public:
	SerialDigitalInput(PinName c, PinName l, PinName d) :
			clk(DigitalOut(c)), load(DigitalOut(l)), data(DigitalIn(d))
	{
		if (clk == NC || load == NC || data == NC)
		{
			fprintf(stderr, "ERROR in SerialDigitalInput: PIN(s) undefined");
			return;
		}

		/*Prepare for next reading*/
		clk = 0;
		load = 1;
	}
	virtual ~SerialDigitalInput()
	{
	}

	uint8_t getBits() const
	{
		return N;
	}

	uint32_t read()
	{
		uint32_t value = 0;
		/*Load the shift register*/
		clk = 0;
		load = 0;

		delay();

		/*Lock the shift register*/
		load = 1;
		for (int i = 0; i < N; i++)
		{
			/*Read value*/
			value = (value << 1) + (data != 0);
			/*Clock. Now the next bit will be loaded*/
			clk = 1;
			/*Delay*/
			delay();
			/*Reset clock*/
			clk = 0;
			/*Delay*/
			delay();
		}
		return value;
	}


private:
	DigitalOut clk;
	DigitalOut load;
	DigitalIn data;

	void delay(){
		uint16_t i=100;
		while(i--);
	}
};

#endif /* TELESCOPE_SERIALDIGITALINPUT_H_ */
