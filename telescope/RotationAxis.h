/*
 * RotationAxis.h
 *
 *  Created on: 2018Äê2ÔÂ11ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_ROTATIONAXIS_H_
#define TELESCOPE_ROTATIONAXIS_H_

#include <StepperMotor.h>
#include <math.h>
#include "mbed.h"
#include "telescope_config.h"

static const float sidereal_speed = 0.00417807462f; /* deg / s */

class RotationAxis
{
public:

	typedef enum
	{
		AXIS_STOPPED = 0,
		AXIS_ACCELERATING,
		AXIS_DECELERATING,
		AXIS_SLEWING,
		AXIS_SLEWING_CONTINUOUS,
		AXIS_TRACKING,
		AXIS_CORRECTING
	} axisstatus_t;

	/**
	 * Define the rotation direction
	 * AXIS_ROTATE_POSITIVE: +angle
	 * AXIS_ROTATE_NEGATIVE: -angle
	 */
	typedef enum
	{
		AXIS_ROTATE_POSITIVE = 0, AXIS_ROTATE_NEGATIVE = 1
	} axisrotdir_t;

	RotationAxis(float stepsPerDeg, StepperMotor *stepper,
	bool invert = false) :
			stepsPerDeg(stepsPerDeg), stepper(stepper), invert(invert), angleDeg(
					0), currentSpeed(0), currentDirection(AXIS_ROTATE_POSITIVE), slewSpeed(
					2), trackSpeed(sidereal_speed), correctionSpeed(
					32 * sidereal_speed), guideSpeed(0.5 * sidereal_speed), acceleration(
					1), status(AXIS_STOPPED), task_thread(osPriorityRealtime,
			OS_STACK_SIZE, NULL, "Axis_task")
	{
		if (stepsPerDeg <= 0)
			error("Axis: steps per degree must be > 0");

		if (!stepper)
			error("Axis: stepper must be defined");

		/*Start the task-handling thread*/
		task_timer.start();
		task_thread.start(callback(this->task, this));
	}

	virtual ~RotationAxis()
	{
	}

	/**
	 * call this function to perform a goto to a specified angle (in Radian) in the specified direction with slewing rate
	 * It will perform an acceleration, a GoTo, and a deceleration before returning
	 */
	bool slewTo(axisrotdir_t dir, float angleDeg)
	{
		msg_t *message = task_pool.alloc();
		if (!message)
		{
			fprintf(stderr, "Axis: out of memory");
			return true;
		}
		message->signal = msg_t::SIGNAL_SLEW_TO;
		message->value = angleDeg;
		message->dir = dir;
		message->tid = Thread::gettid();
		if (task_queue.put(message) != osOK)
		{
			fprintf(stderr, "Axis: failed to queue the operation");
			task_pool.free(message);
			return true;
		}
		/*Wait for the specified action to finish*/
		Thread::signal_clr(0x7FFFFFFF);
		Thread::signal_wait(0);

		return false;
	}

	bool startSlewingContinuous(axisrotdir_t dir)
	{
		msg_t *message = task_pool.alloc();
		if (!message)
		{
			fprintf(stderr, "Axis: out of memory");
			return true;
		}
		message->signal = msg_t::SIGNAL_SLEW_CONT;
		message->value = angleDeg;
		message->dir = dir;
		if (task_queue.put(message) != osOK)
		{
			fprintf(stderr, "Axis: failed to queue the operation");
			task_pool.free(message);
			return true;
		}

		return false;
	}

	/**
	 * call this function to start tracking in tracking rate and specified direction
	 */
	bool startTracking(axisrotdir_t dir)
	{
		msg_t *message = task_pool.alloc();
		if (!message)
		{
			fprintf(stderr, "Axis: out of memory");
			return true;
		}
		message->signal = msg_t::SIGNAL_TRACK_CONT;
		message->dir = dir;
		if (task_queue.put(message) != osOK)
		{
			fprintf(stderr, "Axis: failed to queue the operation");
			task_pool.free(message);
			return true;
		}

		return false;
	}

	/**
	 * Guid on the specified direction for specified time
	 */
	bool guide(axisrotdir_t dir, float time)
	{
		msg_t *message = task_pool.alloc();
		if (!message)
		{
			fprintf(stderr, "Axis: out of memory");
			return true;
		}
		message->signal = msg_t::SIGNAL_GUIDE;
		message->dir = dir;
		message->value = time;
		message->tid = Thread::gettid();
		if (task_queue.put(message) != osOK)
		{
			fprintf(stderr, "Axis: failed to queue the operation");
			task_pool.free(message);
			return true;
		}
		Thread::signal_clr(0x7FFFFFFF);
		Thread::signal_wait(0);

		return false;
	}

	/**
	 * call this function to stop tracking or slewing.
	 * Can be called from another thread or ISR to force stop the slewing
	 */
	bool stop()
	{
		msg_t *message = task_pool.alloc();
		if (!message)
		{
//			fprintf(stderr, "Axis: out of memory");
			return true;
		}
		message->signal = msg_t::SIGNAL_STOP;
		if (task_queue.put(message, 0, 0xFF) != osOK) /*Stop signal has a higher priority*/
		{
//			fprintf(stderr, "Axis: failed to queue the operation");
			task_pool.free(message);
			return true;
		}
		return false;
	}

	void setAngleDeg(float angle)
	{
		angleDeg = angle;
	}

	float getAngleDeg()
	{
		_update_angle_from_step_count();
		return angleDeg;
	}

	axisstatus_t getStatus()
	{
		return status;
	}

	float getAcceleration() const
	{
		return acceleration;
	}

	void setAcceleration(float acceleration)
	{
		if (acceleration <= 0)
		{
			fprintf(stderr, "Axis: acceleration must be >0");
		}
		else
		{
			this->acceleration = acceleration;
		}
	}

	float getSlewSpeed() const
	{
		return slewSpeed;
	}

	void setSlewSpeed(float slewSpeed)
	{
		if (slewSpeed <= 0)
		{
			fprintf(stderr, "Axis: slew speed must be >0");
		}
		else
		{
			this->slewSpeed = slewSpeed;
		}
	}

	float getTrackSpeedSidereal() const
	{
		return trackSpeed / sidereal_speed;
	}

	void setTrackSpeedSidereal(float trackSpeed)
	{
		if (trackSpeed <= 0)
		{
			fprintf(stderr, "Axis: track speed must be >0");
		}
		else
		{
			this->trackSpeed = trackSpeed * sidereal_speed;
		}
	}

	float getGuideSpeedSidereal() const
	{
		return guideSpeed / sidereal_speed;
	}

	void setGuideSpeedSidereal(float guideSpeed)
	{
		if (guideSpeed <= 0 || guideSpeed >= trackSpeed)
		{
			fprintf(stderr, "Axis: guide speed must be > 0 and < track speed");
		}
		else
		{
			this->guideSpeed = guideSpeed * sidereal_speed;
		}
	}

	float getCorrectionSpeedSidereal() const
	{
		return correctionSpeed / sidereal_speed;
	}

	void setCorrectionSpeedSidereal(float correctionSpeed)
	{
		if (correctionSpeed <= 0)
		{
			fprintf(stderr, "Axis: correction speed must be >0");
		}
		else
		{
			this->correctionSpeed = correctionSpeed * sidereal_speed;
		}
	}

	float getCurrentSpeed() const
	{
		return currentSpeed;
	}

protected:

	typedef struct
	{
		enum sig_t
		{
			SIGNAL_SLEW_TO = 0,
			SIGNAL_SLEW_CONT,
			SIGNAL_TRACK_CONT,
			SIGNAL_GUIDE,
			SIGNAL_STOP
		} signal;
		float value;
		axisrotdir_t dir;
		osThreadId_t tid;
	} msg_t;

	/*Configurations*/
	float stepsPerDeg; /*speedFactor * speed(deg/s) = step per second (Hz)*/
	StepperMotor *stepper; /*Pointer to stepper motor*/
	bool invert;

	/*Runtime values*/
	volatile float angleDeg;
	volatile float currentSpeed; /*Current speed in deg/s*/
	volatile axisrotdir_t currentDirection;
	float slewSpeed; /* Slewing speed in deg/s */
	float trackSpeed; /* Tracking speed in deg/s (no accel/deceleration) */
	float correctionSpeed; /* Correction speed in deg/s (no accel/deceleration) */
	float guideSpeed; /* Guide speed in deg/s. this amount will be subtracted/added to the trackSpeed, and so must be less than track speed */
	float acceleration; /* Acceleration in deg/s^2*/
	volatile axisstatus_t status;
	Thread task_thread; /*Thread for executing all lower-level tasks*/
	Queue<msg_t, 16> task_queue; /*Queue of messages*/
	MemoryPool<msg_t, 16> task_pool; /*MemoryPool for allocating messages*/
	Timer task_timer;

	static void task(RotationAxis *p);

	/*Low-level functions for internal use*/
	void _slewto(axisrotdir_t dir, float dest);
	void _slewcont(axisrotdir_t dir);
	void _track(axisrotdir_t dir);
	void _guide(axisrotdir_t dir, float duration);
	void _update_angle_from_step_count();
	void _decel_stop();
	void _stop();
};

#endif /* TELESCOPE_ROTATIONAXIS_H_ */
