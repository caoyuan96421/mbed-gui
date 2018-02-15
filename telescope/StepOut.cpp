/*
 #include <StepOut.h>
 * ControllablePWMOut.cpp
 *
 *  Created on: 2018Äê2ÔÂ9ÈÕ
 *      Author: caoyuan9642
 */

#include "mbed.h"
#include <StepOut.h>

void StepOut::start()
{
	if (status == IDLE)
	{
		core_util_critical_section_enter();
		status = STEPPING;
		this->write(0.5);
		tim.reset();
		core_util_critical_section_exit();
	}
}

void StepOut::stop()
{
	if (status == STEPPING)
	{
		core_util_critical_section_enter();
		status = IDLE;
		this->write(0);
		stepCount += (uint64_t) (tim.read() * freq);
		core_util_critical_section_exit();
	}
}

float StepOut::setPeriod(float period)
{
	if (period > 0.0f)
	{
		int us_period = ceilf(period * 1e6); /*Ceil to the next microsecond*/
		if (status == IDLE)
			this->period_us(us_period);
		else
		{
			core_util_critical_section_enter();
			stop(); /*Stop to correctly update the stepCount*/
			this->period_us(us_period);
			start();
			core_util_critical_section_exit();
		}
		freq = 1000000.0 / us_period; // get CORRECT frequency!
		return us_period / 1e6; // Return the accurate period
	}
	else
	{
		return 0;
	}
}

void StepOut::resetCount()
{
	core_util_critical_section_enter();
	stepCount = 0;
	if (status == STEPPING)
		tim.reset();
	core_util_critical_section_exit();
}

uint64_t StepOut::getCount()
{
	if (status == IDLE)
		return stepCount;
	else
	{
		return stepCount + (uint64_t) (freq * tim.read()); /*Estimate count at now*/
	}
}
