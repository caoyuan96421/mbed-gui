/*
 * OSWrapper.cpp
 *
 *  Created on: 2018Äê2ÔÂ3ÈÕ
 *      Author: caoyuan9642
 */

#include "mbed.h"
#include "rtos.h"
#include <touchgfx/hal/OSWrappers.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include <touchgfx/hal/HAL.hpp>

using namespace touchgfx;

static Semaphore frame_buffer_sem(1, 1);
static Semaphore vsync_sem(0, 1);


void OSWrappers::initialize() {
}

void OSWrappers::takeFrameBufferSemaphore() {
	frame_buffer_sem.wait();
}
void OSWrappers::giveFrameBufferSemaphore() {
	frame_buffer_sem.release();
}

void OSWrappers::tryTakeFrameBufferSemaphore() {
	frame_buffer_sem.wait(0);
}

void OSWrappers::giveFrameBufferSemaphoreFromISR() {
	frame_buffer_sem.release();
}

void OSWrappers::signalVSync() {
	vsync_sem.release();
}

void OSWrappers::waitForVSync() {
	vsync_sem.wait(0); /*Clear any pending VSYNC first*/
	vsync_sem.wait(); /*Then wait for the next signal*/
}

void OSWrappers::taskDelay(uint16_t ms) {
	osDelay(ms);
}
