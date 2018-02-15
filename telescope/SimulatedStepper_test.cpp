/*
 * SimulatedStepper_test.cpp
 *
 *  Created on: 2018Äê2ÔÂ11ÈÕ
 *      Author: caoyuan9642
 */

#include <SimulatedStepper_HW.h>
#include <RotationAxis.h>

Thread report;
SimulatedStepper_HW stepper(D2, D3);
const float stepsPerDeg = 10000.0f;
RotationAxis axis(stepsPerDeg, &stepper);

extern void xprintf(const char *, ...);

void report_stepper()
{
	while (1)
	{
		xprintf("Position: %f (%f) deg. Status: %d. Speed: %f",
				axis.getAngleDeg(), stepper.getStepCountHW() / stepsPerDeg,
				axis.getStatus(), axis.getCurrentSpeed());
		wait(0.1);
	}
}

void emerg_stop(RotationAxis *pa)
{
	pa->stop();
}

void test_stepper()
{
	Timeout to, t2;
	report.start(report_stepper);

	// Test 1
	wait(2);
	xprintf("Test 1: Slew to -10deg at 1 deg/s");

	axis.slewTo(RotationAxis::AXIS_ROTATE_NEGATIVE, -10);

	// Test 2
	wait(2);
	xprintf("Test 2: Slew to 10deg at 4 deg/s");

	axis.setSlewSpeed(4);
	axis.slewTo(RotationAxis::AXIS_ROTATE_POSITIVE, 10);

	// Test 3
	wait(2);
	xprintf(
			"Test 3: Slew to 20deg at 4 deg/s and emergency stop after 5s, then to 0 degree");
	axis.setSlewSpeed(4);
	to.attach(callback(emerg_stop, &axis), 5);
	t2.attach(callback(emerg_stop, &axis), 4.7);
//	to.attach(callback(emerg_stop, &axis), 10);
	axis.slewTo(RotationAxis::AXIS_ROTATE_POSITIVE, 20);
	axis.slewTo(RotationAxis::AXIS_ROTATE_NEGATIVE, 0);

	// Test 4
	wait(2);
	xprintf("Test 4: Slew to 2deg at 4 deg/s");
	axis.setSlewSpeed(4);
	axis.slewTo(RotationAxis::AXIS_ROTATE_POSITIVE, 2);

	// Test 5
	wait(2);
	xprintf("Test 5: Slew to 1.8deg");
	axis.setSlewSpeed(4);
	axis.slewTo(RotationAxis::AXIS_ROTATE_NEGATIVE, 1.8);

	// Test 6
	wait(2);
	xprintf("Test 6: Slew to 1.2deg");
	axis.setSlewSpeed(4);
	axis.slewTo(RotationAxis::AXIS_ROTATE_NEGATIVE, 1.2);

	// Test 7
	wait(2);
	xprintf("Test 7: Slew to 0deg");
	axis.setSlewSpeed(4);
	axis.slewTo(RotationAxis::AXIS_ROTATE_NEGATIVE, 0);

	// Test 8
	wait(2);
	xprintf("Test 8: Slew for 15s in async mode");
	axis.setSlewSpeed(4);
	axis.setAcceleration(4);
	axis.startSlewingContinuous(RotationAxis::AXIS_ROTATE_POSITIVE);

	wait(15);

	axis.stop();

	wait(2);
	axis.slewTo(RotationAxis::AXIS_ROTATE_NEGATIVE, 0);

	xprintf("Test 9: Guiding");
	wait(2);
	axis.startTracking(RotationAxis::AXIS_ROTATE_POSITIVE);

	for (int i = 1; i <= 10; i++)
	{
		axis.guide(RotationAxis::AXIS_ROTATE_POSITIVE, (float) i / 10 * 4);
		wait(1);
		axis.guide(RotationAxis::AXIS_ROTATE_NEGATIVE, (float) i / 10 * 4);
		wait(1);
	}

	axis.stop();

	wait(1);

	xprintf("Test finished");
	report.terminate();

}
