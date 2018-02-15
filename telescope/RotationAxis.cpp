/*
 * RotationAxis.cpp
 *
 *  Created on: 2018Äê2ÔÂ11ÈÕ
 *      Author: caoyuan9642
 */

#include <RotationAxis.h>
#include <StepperMotor.h>
#include <telescope_config.h>
#include "mbed.h"

extern void xprintf(const char *, ...);

void RotationAxis::task(RotationAxis *p)
{

	/*Main loop of RotationAxis*/
	while (true)
	{
		/*Get next message*/
		msg_t *message;
		enum msg_t::sig_t signal;
		float value;
		axisrotdir_t dir;
		osThreadId_t tid;

		osEvent evt = p->task_queue.get(AXIS_UPDATE_TIMEOUT);
		if (evt.status == osEventTimeout)
		{
			/*Timeout when fetching from the queue. Just update the angle*/
			p->_update_angle_from_step_count();
			continue;
		}
		else if (evt.status == osEventMessage)
		{
			/*New message arrived. Copy the data and free is asap*/
			message = (msg_t*) evt.value.p;
			signal = message->signal;
			value = message->value;
			dir = message->dir;
			tid = message->tid;
			p->task_pool.free(message);
		}
		else
		{
			/*Error*/
			fprintf(stderr, "Axis: Error fetching the task queue.");
			continue;
		}

		/*Check the type of the signal*/
		switch (signal)
		{
		case msg_t::SIGNAL_SLEW_TO:
			if (p->status == AXIS_STOPPED)
			{
				p->_slewto(dir, value);
			}
			else
			{
				fprintf(stderr,
						"Axis: being slewed while not in STOPPED mode. ");
			}
			osThreadFlagsSet(tid, 0x01); /*Send a signal so that the caller is free to run*/
			break;
		case msg_t::SIGNAL_SLEW_CONT:
			if (p->status == AXIS_STOPPED)
			{
				p->_slewcont(dir);
			}
			else
			{
				fprintf(stderr,
						"Axis: being slewed while not in STOPPED mode. ");
			}
			break;
		case msg_t::SIGNAL_TRACK_CONT:
			if (p->status == AXIS_STOPPED)
			{
				p->_track(dir);
			}
			else
			{
				fprintf(stderr,
						"Axis: trying to track while not in STOPPED mode. ");
			}
			break;
		case msg_t::SIGNAL_GUIDE:
			if (p->status == AXIS_TRACKING)
			{
				p->_guide(dir, value);
			}
			else
			{
				fprintf(stderr,
						"Axis: trying to guide while not in TRACKING mode. ");
			}
			osThreadFlagsSet(tid, 0x01); /*Send a signal so that the caller is free to run*/
			break;
		case msg_t::SIGNAL_STOP:
			if (p->status == AXIS_SLEWING
					|| p->status == AXIS_SLEWING_CONTINUOUS)
			{
				p->_decel_stop();
			}
			else if (p->status == AXIS_TRACKING)
			{
				p->_stop();
			}
			p->status = AXIS_STOPPED;
			break;
		default:
			fprintf(stderr, "Axis: undefined signal %d", message->signal);
		}

		/*Update the angle at the end of each operation*/
		p->_update_angle_from_step_count();
	}
}

void RotationAxis::_stop()
{
	stepper->stop();
}

void RotationAxis::_slewto(axisrotdir_t dir, float dest)
{
	if (isnan(dest) || isinf(dest) || dest > 360 || dest < -360)
	{
		fprintf(stderr,
				"Axis: invalid angle. Must be within -360..360 degrees");
		return;
	}

	_update_angle_from_step_count();

	bool emergency_stopped = false;
	float delta;
	delta = (dest - angleDeg) * (dir == AXIS_ROTATE_POSITIVE ? 1 : -1); /*delta is the actual angle to rotate*/
	delta = delta - floor(delta / 360.0f) * 360.0f; /*Shift to 0-360 deg*/

	StepperMotor::stepdir_t sd = (StepperMotor::stepdir_t) (dir ^ invert);

	xprintf("Axis: delta=%f", delta); //TODO: DEBUG

	status = AXIS_SLEWING;
	currentDirection = dir;

	if (delta > AXIS_MINIMUM_SLEW_ANGLE)
	{
		delta = delta - 0.5 * AXIS_MINIMUM_SLEW_ANGLE; /*The motion angle is decreased to ensure the correction step is in the same direction*/

		float endspeed = slewSpeed;
		float waitTime = delta / slewSpeed - slewSpeed / acceleration; /*Calculate theoretical wait time assuming uniform acceleration and deceleration*/
		if (waitTime < 0.0f)
		{
			/*We need to decrease the endspeed so that waitTime will be zero*/
			waitTime = 0.0f;
			/*delta = endspeed^2 / acceleration*/
			endspeed = sqrt(delta * acceleration);
		}

		xprintf("Axis: speed = %f deg/s, time=%f, acc=%f", endspeed, waitTime,
				acceleration); //TODO: DEBUG

		/*Accelerate to slew speed*/

		unsigned int ramp_steps = (unsigned int) (endspeed
				/ AXIS_ACCELERATION_STEP_TIME / acceleration);

		if (ramp_steps < 1)
			ramp_steps = 1;

		xprintf("Axis: accelerate in %d steps", ramp_steps); //TODO: DEBUG

		for (unsigned int i = 1; i <= ramp_steps; i++)
		{

			currentSpeed = stepper->setFrequency(
					stepsPerDeg * endspeed / ramp_steps * i) / stepsPerDeg; // Set and update currentSpeed with actual speed

			if (i == 1)
				stepper->start(sd);

			/*Monitor whether there is a stop signal*/
			osEvent ev = task_queue.get(
					(int) (AXIS_ACCELERATION_STEP_TIME * 1000));

			if (ev.status == osEventTimeout)
			{
				/*Nothing happened, we're good*/
				_update_angle_from_step_count();
				continue;
			}
			else if (ev.status == osEventMessage)
			{
				msg_t *message = (msg_t*) ev.value.p;
				enum msg_t::sig_t signal = message->signal;
				task_pool.free(message); /*We will discard whatever message there is*/
				if (signal == msg_t::SIGNAL_STOP)
				{
					/*We're stopped. Break out*/
					emergency_stopped = true;
					break;
				}
			}
		}

		if (!emergency_stopped)
		{
			/*Keep slewing*/
			xprintf("Axis: accelerate done"); //TODO: DEBUG

			float waitUntil = waitTime + task_timer.read();
			float timeNow;

			while ((timeNow = task_timer.read()) < waitUntil)
			{
				osEvent ev = task_queue.get(
						(int) floor((waitUntil - timeNow) * 1000)); /*Wait the remaining time*/
				if (ev.status == osEventTimeout)
				{
					/*Nothing happened, we're good*/
					_update_angle_from_step_count();
					continue;
				}
				else if (ev.status == osEventMessage)
				{
					msg_t *message = (msg_t*) ev.value.p;
					enum msg_t::sig_t signal = message->signal;
					task_pool.free(message); /*We will discard whatever message there is*/
					if (signal == msg_t::SIGNAL_STOP)
					{
						/*We're stopped. Break out*/
						emergency_stopped = true;
						break;
					}
				}
			}
		}

		_decel_stop();
	}

	if (!emergency_stopped)
	{
		/*Use correction to goto the final angle with high resolution*/
		_update_angle_from_step_count();
		xprintf("Axis: correct from %f to %f deg", angleDeg, dest); // TODO: DEBUG

		float diff = remainderf(angleDeg - dest, 360);
		if (diff > AXIS_CORRECTION_MAX_ANGLE)
		{
			fprintf(stderr,
					"Axis: correction too large: %f. Check hardware configuration.",
					diff);
			status = AXIS_STOPPED;
			return;
		}

		int nTry = 3; // Try 3 corrections at most
		while (--nTry && fabsf(diff) > AXIS_CORRECTION_ANGLE_TOLERANCE)
		{
			/*Determine correction direction and time*/
			sd = (StepperMotor::stepdir_t) ((diff > 0.0f) ^ invert);

			/*Perform correction*/
			currentSpeed = stepper->setFrequency(stepsPerDeg * correctionSpeed)
					/ stepsPerDeg; // Set and update actual speed

			float correctionTime = (float) (fabsf(diff)) / currentSpeed; // Use the accurate speed for calculating time

			xprintf("Axis: correction: from %f to %f deg. time=%f", angleDeg,
					dest, correctionTime); //TODO: DEBUG
			if (correctionTime < AXIS_CORRECTION_MIN_TIME)
			{
				break;
			}

			/*Start, wait, stop*/
			stepper->start(sd);
			wait(correctionTime);
			stepper->stop();

			currentSpeed = 0;
			_update_angle_from_step_count();
			diff = remainderf(angleDeg - dest, 360);
		}

		if (!nTry)
		{
			fprintf(stderr,
					"Axis: correction failed. Check hardware configuration.");
		}

		xprintf("Axis: correction finished: %f deg", angleDeg); //TODO:DEBUG
	}
	status = AXIS_STOPPED;
}

void RotationAxis::_slewcont(axisrotdir_t dir)
{
	/*Continuous mode*/
	StepperMotor::stepdir_t sd = (StepperMotor::stepdir_t) (dir ^ invert);

	status = AXIS_SLEWING_CONTINUOUS;
	currentDirection = dir;

	bool emergency_stopped = false;
	unsigned int ramp_steps = (unsigned int) (slewSpeed
			/ AXIS_ACCELERATION_STEP_TIME / acceleration);

	if (ramp_steps < 1)
		ramp_steps = 1;

	xprintf("Axis: accelerate in %d steps", ramp_steps); //TODO: DEBUG

	for (unsigned int i = 1; i <= ramp_steps; i++)
	{

		currentSpeed = stepper->setFrequency(
				stepsPerDeg * slewSpeed / ramp_steps * i) / stepsPerDeg; // set and update actual speed

		if (i == 1)
			stepper->start(sd);

		/*Monitor whether there is a stop signal*/
		osEvent ev = task_queue.get((int) (AXIS_ACCELERATION_STEP_TIME * 1000));

		if (ev.status == osEventTimeout)
		{
			/*Nothing happened, we're good*/
			continue;
		}
		else if (ev.status == osEventMessage)
		{
			msg_t *message = (msg_t*) ev.value.p;
			enum msg_t::sig_t signal = message->signal;
			task_pool.free(message); /*We will discard whatever message there is*/
			if (signal == msg_t::SIGNAL_STOP)
			{
				/*We're stopped. Break out*/
				emergency_stopped = true;
				break;
			}
		}
	}
	if (emergency_stopped)
	{
		_decel_stop();
		status = AXIS_STOPPED;
	}
}

void RotationAxis::_track(axisrotdir_t dir)
{
	StepperMotor::stepdir_t sd = (StepperMotor::stepdir_t) (dir ^ invert);
	currentSpeed = stepper->setFrequency(trackSpeed * stepsPerDeg)
			/ stepsPerDeg;
	currentDirection = dir;
	stepper->start(sd);
	status = AXIS_TRACKING;
}

void RotationAxis::_guide(axisrotdir_t dir, float duration)
{
	if (duration > AXIS_MAX_GUIDE_TIME)
	{
		fprintf(stderr, "Axis: Guiding time too long: %f seconds", duration);
		return;
	}

	currentSpeed =
			(currentDirection == dir) ?
					trackSpeed + guideSpeed : trackSpeed - guideSpeed; /*Determine speed based on direction*/

	currentSpeed = stepper->setFrequency(currentSpeed * stepsPerDeg)
			/ stepsPerDeg; //set and update accurate speed

	wait(duration);

	currentSpeed = stepper->setFrequency(trackSpeed * stepsPerDeg)
			/ stepsPerDeg;
}

void RotationAxis::_update_angle_from_step_count()
{
	/*This should in principle be thread-safe and interrupt-safe, since reading the step count does not change internal state*/
	angleDeg = remainderf((float) stepper->getStepCount() / stepsPerDeg, 360);

	/*However, for implementing a hardware (encoder) based readout, some synchronization must be used*/
}

void RotationAxis::_decel_stop()
{
	/*Decelerate and stop*/
	float endspeed = currentSpeed;
	unsigned int ramp_steps = (unsigned int) (currentSpeed
			/ AXIS_ACCELERATION_STEP_TIME / acceleration);

	if (ramp_steps < 1)
		ramp_steps = 1;

	xprintf("Axis: decelerate in %d steps", ramp_steps); // TODO: DEBUG

	for (unsigned int i = ramp_steps - 1; i >= 1; i--)
	{
		currentSpeed = stepper->setFrequency(
				stepsPerDeg * endspeed / ramp_steps * i) / stepsPerDeg; // set and update accurate speed
		wait(AXIS_ACCELERATION_STEP_TIME);
		_update_angle_from_step_count();
	}

	stepper->stop();
	currentSpeed = 0;
}
