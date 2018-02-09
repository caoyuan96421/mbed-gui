/*
 * AMIS30543StepperDriver.cpp
 *
 *  Created on: 2018Äê2ÔÂ8ÈÕ
 *      Author: caoyuan9642
 */

#include <driver/AMIS30543StepperDriver.h>
#include "mbed.h"
#include "pinmap.h"

typedef enum {
	WR = 0,
	CR0 = 1,
	CR1 = 2,
	CR2 = 3,
	SR0 = 4,
	SR1 = 5,
	SR2 = 6,
	SR3 = 7,
	SR4 = 8,
	CR3 = 9
} regaddr_t;

static char txbuf[2], rxbuf[2];
static void writeReg(SPI *spi, regaddr_t addr, uint8_t data) {
	txbuf[0] = (char) (addr & 0x1F) | 0x80;
	txbuf[1] = data;
	spi->write(txbuf, 2, NULL, 0);
}

static uint8_t readReg(SPI *spi, regaddr_t addr) {
	txbuf[0] = (char) addr;
	spi->write(txbuf, 2, rxbuf, 2);
	return rxbuf[1];
}

extern void xprintf(const char *, ...);

AMIS30543StepperDriver::AMIS30543StepperDriver(PinName mosi, PinName miso,
		PinName sclk, PinName step, PinName c, PinName d, PinName e) :
		SPI(mosi, miso, sclk), cs(DigitalOut(c)), nextstep(DigitalOut(step)), dir(
				DigitalOut(d)), err(DigitalIn(e, OpenDrain)) { // Force use software CS
	if (c != NC) {
		useCS = true;
		this->cs = 1; /*deassert CS*/
	} else
		useCS = false;
	if (d != NC) {
		useDIR = true;
		this->dir = 0; /*Forward*/
	} else
		useDIR = false;
	if (e != NC) {
		useERR = true;
	} else
		useERR = false;

	frequency(200000);

	/*CPOL=0, CPHA=0*/
	format(8, 0);

	/*set pull up on SPI pins*/
	pin_mode(_spi.spi.pin_miso, PullUp);

	/*Reset stepping pin*/
	nextstep = 0;

	/*Perform initialization*/
	lock();
	assertCS();
	wait_us(2);

	writeReg(this, CR2, 0x80); // MOTEN = 1

	wait_us(2);
	deassertCS();
	wait_us(10);
	assertCS();
	wait_us(2);

	writeReg(this, CR0, 0x0A); // Step mode 1/32, motor current 0.78A

	wait_us(2);
	deassertCS();
	wait_us(10);
	assertCS();
	wait_us(2);

	writeReg(this, CR3, 0x01); // Step mode 1/128

	wait_us(2);
	deassertCS();
	wait_us(10);
	assertCS();
	wait_us(2);

	xprintf("Reg CR0: 0x%2x", readReg(this, CR0));

	wait_us(2);
	deassertCS();
	wait_us(10);
	assertCS();
	wait_us(2);

	xprintf("Reg CR1: 0x%2x", readReg(this, CR1));

	wait_us(2);
	deassertCS();
	wait_us(10);
	assertCS();
	wait_us(2);

	xprintf("Reg CR2: 0x%2x", readReg(this, CR2));

	wait_us(2);
	deassertCS();
	wait_us(10);
	assertCS();
	wait_us(2);

	xprintf("Reg CR3: 0x%2x", readReg(this, CR3));

	wait_us(2);
	deassertCS();
	unlock();
}

AMIS30543StepperDriver::~AMIS30543StepperDriver() {
}

void AMIS30543StepperDriver::step() {
	nextstep = 1;
	wait_us(3);
	nextstep = 0;
	wait_us(3);
}

void AMIS30543StepperDriver::setStepDirection(stepdir_t d) {
	GenericStepperMotor::setStepDirection(d);
	if (useDIR) {
		if (d == STEP_FORWARD)
			dir = 0;
		else
			dir = 1;
	} else {
		lock();
		assertCS();
		wait_us(2);

		uint8_t cr1 = readReg(this, CR1) & 0x7F;

		wait_us(2);
		deassertCS();

		if (d == STEP_FORWARD)
			cr1 |= 0x00;
		else
			cr1 |= 0x80;

		wait_us(5);
		assertCS();
		wait_us(2);

		writeReg(this, CR1, cr1);

		wait_us(2);

		deassertCS();
		unlock();
	}
}

void AMIS30543StepperDriver::assertCS() {
	if (useCS) {
		cs = 0;
	}
}

void AMIS30543StepperDriver::deassertCS() {
	if (useCS) {
		cs = 1;
	}
}
