/*
 * AMIS30543StepperDriver.cpp
 *
 *  Created on: 2018Äê2ÔÂ8ÈÕ
 *      Author: caoyuan9642
 */

#include <AMIS30543StepperDriver.h>
#include "mbed.h"
#include "pinmap.h"

void AMIS30543StepperDriver::writeReg(regaddr_t addr, uint8_t data)
{
	assertCS();
	wait_us(3);
	txbuf[0] = (char) (addr & 0x1F) | 0x80;
	txbuf[1] = data;
	spi->write(txbuf, 2, NULL, 0);
	wait_us(3);
	deassertCS();
	wait_us(6);
}

void AMIS30543StepperDriver::start(stepdir_t d)
{
	/*Set Direction*/
	if (useDIR)
	{
		if (d == STEP_FORWARD)
			dir = 0;
		else
			dir = 1;
	}
	else
	{
		uint8_t cr1 = readReg(CR1) & 0x7F;
		if (d == STEP_FORWARD)
			cr1 |= 0x00;
		else
			cr1 |= 0x80;
		writeReg(CR1, cr1);
	}

	step.start();
}

void AMIS30543StepperDriver::stop()
{
	step.stop();
}

void AMIS30543StepperDriver::stepN(stepdir_t d, uint64_t N)
{
	float timeRequired = (float) N * period;
	start(d);
	wait(timeRequired);
	stop();
}

void AMIS30543StepperDriver::setPeriod(float period)
{
	this->period = period;
	step.setPeriod(period);
}

void AMIS30543StepperDriver::setCurrent(float current)
{
}

uint8_t AMIS30543StepperDriver::readReg(regaddr_t addr)
{
	assertCS();
	wait_us(3);
	txbuf[0] = (char) (addr & 0x1F);
	spi->write(txbuf, 2, rxbuf, 2);
	wait_us(3);
	deassertCS();
	wait_us(6);
	return rxbuf[1];
}

extern void xprintf(const char *, ...);

AMIS30543StepperDriver::AMIS30543StepperDriver(SPI *spi, PinName step,
		PinName c, PinName d, PinName e) :
		spi(spi), cs(DigitalOut(c)), dir(DigitalOut(d)), err(
				DigitalIn(e, OpenDrain))
{
	static StepOut s(step);
	this->step = s;
	if (d != NC)
	{
		useDIR = true;
		this->dir = 0; /*Forward*/
	}
	else
		useDIR = false;
	if (e != NC)
	{
		useERR = true;
	}
	else
		useERR = false;

	spi->frequency(400000);

	/*CPOL=0, CPHA=0*/
	spi->format(8, 0);

	/*Perform initialization*/
	writeReg(CR2, 0x80); // MOTEN = 1
	writeReg(CR0, 0x0A); // Step mode 1/32, motor current 0.78A
	writeReg(CR3, 0x01); // Step mode 1/128

	xprintf("Reg CR0: 0x%2x", readReg(CR0));
	xprintf("Reg CR1: 0x%2x", readReg(CR1));
	xprintf("Reg CR2: 0x%2x", readReg(CR2));
	xprintf("Reg CR3: 0x%2x", readReg(CR3));
}

inline void AMIS30543StepperDriver::assertCS()
{
	cs = 0;
}

inline void AMIS30543StepperDriver::deassertCS()
{
	cs = 1;
}
