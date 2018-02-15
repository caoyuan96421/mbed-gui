/*
 * telescope_config.h
 *
 *  Created on: 2018Äê2ÔÂ11ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_CONFIG_TELESCOPE_CONFIG_H_
#define TELESCOPE_CONFIG_TELESCOPE_CONFIG_H_

/**
 * Time increments in axis acceleration. Unit: s
 */
#define AXIS_ACCELERATION_STEP_TIME	0.1f

/**
 * Minimum slew angle. Angles below this value will be reached by correction steps (i.e. no accel/deceleration)
 */
#define AXIS_MINIMUM_SLEW_ANGLE 0.3f

/**
 * Tolerance for GoTo pointing corrections in degrees. Default to 0.05 (3 arcmin)
 */
#define AXIS_CORRECTION_ANGLE_TOLERANCE 0.05f

/**
 * Minimum time of the pointing correction. If the correction requires less than this value, it will be ignored
 */
#define AXIS_CORRECTION_MIN_TIME 0.005f

/**
 * Maximum allowed correction angle. If this angle is exceeded, it means there is something wrong with the mount, either the motor is jamed or encoder is broken
 */
#define AXIS_CORRECTION_MAX_ANGLE 1.0f


/**
 * Default timeout for updating the axis position. In milliseconds
 */
#define AXIS_UPDATE_TIMEOUT 10

/**
 * Maximum guide time in seconds
 */
#define AXIS_MAX_GUIDE_TIME 5

#endif /* TELESCOPE_CONFIG_TELESCOPE_CONFIG_H_ */
