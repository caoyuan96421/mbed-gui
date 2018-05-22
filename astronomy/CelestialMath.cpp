/*
 * CelestialMath.cpp
 *
 *  Created on: Feb 21, 2018
 *      Author: Yuan
 */

#include "CelestialMath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mbed.h"

#define CM_DEBUG 0

static inline double clamp(double x)
{
	return (x > 1) ? 1 : ((x < -1) ? -1 : x);
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_ITERATION 30
#define MAX_ITERATION_OPTIMIZATION 10

static const double tol = 1e-10;
static const double eps = 1e-13;
static const double delta = 1e-7;

static const double RADIAN = 180.0 / M_PI;
static const double DEGREE = M_PI / 180.0;

CartesianVector CartesianVector::operator*(const Transformation &t)
{
	return CartesianVector(t.a11 * x + t.a21 * y + t.a31 * z, t.a12 * x + t.a22 * y + t.a32 * z, t.a13 * x + t.a23 * y + t.a33 * z);
}

CartesianVector Transformation::operator*(const CartesianVector &vec)
{ // Left-product of matrix and vector
	return CartesianVector(a11 * vec.x + a12 * vec.y + a13 * vec.z, a21 * vec.x + a22 * vec.y + a23 * vec.z, a31 * vec.x + a32 * vec.y + a33 * vec.z);
}

AzimuthalCoordinates CelestialMath::localEquatorialToAzimuthal(const LocalEquatorialCoordinates &a, const LocationCoordinates &loc)
{
	//              cphi             lambda             lambda0
	double c1 = cos(a.ha * DEGREE), c2 = cos(a.dec * DEGREE), c3 = cos(loc.lat * DEGREE);
	double s1 = sin(a.ha * DEGREE), s2 = sin(a.dec * DEGREE), s3 = sin(loc.lat * DEGREE);
	double s4 = c1 * c2 * c3 + s2 * s3;
	s4 = clamp(s4);
	double y1 = s1 * c2, x1 = s2 * c3 - c1 * c2 * s3;

	return AzimuthalCoordinates(asin(clamp(s4)) * RADIAN, atan2(y1, x1) * RADIAN);
}

LocalEquatorialCoordinates CelestialMath::azimuthalToLocalEquatorial(const AzimuthalCoordinates &b, const LocationCoordinates &loc)
{
	//              mu             eps             lambda0
	double c1 = cos(b.azi * DEGREE), c2 = cos(b.alt * DEGREE), c3 = cos(loc.lat * DEGREE);
	double s1 = sin(b.azi * DEGREE), s2 = sin(b.alt * DEGREE), s3 = sin(loc.lat * DEGREE);
	double s4 = c1 * c2 * c3 + s2 * s3;
	double y1 = s1 * c2, x1 = s2 * c3 - c1 * c2 * s3;

	return LocalEquatorialCoordinates(asin(clamp(s4)) * RADIAN, atan2(y1, x1) * RADIAN);
}

LocalEquatorialCoordinates AlignmentStar::star_ref_local(const LocationCoordinates &loc) const
{
	return CelestialMath::equatorialToLocalEquatorial(star_ref, timestamp, loc);
}

double CelestialMath::getGreenwichMeanSiderealTime(time_t timestamp)
{
	double jd = (double) timestamp * 1.1574074074074E-5 + 2440587.5 - 2451545.0; // Julian Date since J2000
//	double jd0 = floor(jd - 0.5) + 0.5; // JD of previous midnight
//	double cent = (jd - 2451545.0) / 36525;
//	double gmst = 6.697374558 + 0.06570982441908 * (jd0 - 2451545.0)
//			+ 1.00273790935 * (jd - jd0) * 24 + 0.000026 * cent * cent;
//	gmst *= 15.0;
	double gmst = 280.46061837 + 360.985647366 * jd; // Greenwich mean sidereal time (angle)
	return remainder(gmst, 360.0);
}

double CelestialMath::getLocalSiderealTime(time_t timestamp, const LocationCoordinates &loc)
{
	double gmst = getGreenwichMeanSiderealTime(timestamp);
	double lst = gmst + loc.lon * 1.00273790935; // Local sidereal time (angle)
	return remainder(lst, 360.0);
}

LocalEquatorialCoordinates CelestialMath::equatorialToLocalEquatorial(const EquatorialCoordinates &e, time_t timestamp, const LocationCoordinates &loc)
{
// From phi to cphi
	return LocalEquatorialCoordinates(e.dec, remainder(getLocalSiderealTime(timestamp, loc) - e.ra, 360.0));
}

EquatorialCoordinates CelestialMath::localEquatorialToEquatorial(const LocalEquatorialCoordinates &a, time_t timestamp, const LocationCoordinates &loc)
{
// From cphi to phi
	return EquatorialCoordinates(a.dec, remainder(getLocalSiderealTime(timestamp, loc) - a.ha, 360.0));
}

Transformation &CelestialMath::getMisalignedPolarAxisTransformation(Transformation &t, const AzimuthalCoordinates &p, const LocationCoordinates &loc)
{
	double c1 = cos(p.azi * DEGREE), c2 = cos(p.alt * DEGREE), c3 = cos(loc.lat * DEGREE);
	double s1 = sin(p.azi * DEGREE), s2 = sin(p.alt * DEGREE), s3 = sin(loc.lat * DEGREE);
// Matrix to convert from basis vectors in misaligned PA to correct PA
	t.a11 = c1 * s2 * s3 + c2 * c3;
	t.a12 = -s1 * s3;
	t.a13 = -c1 * c2 * s3 + s2 * c3;
	t.a21 = s1 * s2;
	t.a22 = c1;
	t.a23 = -s1 * c2;
	t.a31 = -c1 * s2 * c3 + c2 * s3;
	t.a32 = s1 * c3;
	t.a33 = c1 * c2 * c3 + s2 * s3;
	return t;
}

LocalEquatorialCoordinates CelestialMath::applyMisalignment(const Transformation &t, const LocalEquatorialCoordinates& a)
{
	double c1 = cos(a.dec * DEGREE), c2 = cos(a.ha * DEGREE);
	double s1 = sin(a.dec * DEGREE), s2 = sin(a.ha * DEGREE);
	CartesianVector X = CartesianVector(c1 * c2, -c1 * s2, s1) * t;

	return LocalEquatorialCoordinates(asin(clamp(X.z)) * RADIAN, atan2(-X.y, X.x) * RADIAN);
}

LocalEquatorialCoordinates CelestialMath::deapplyMisalignment(const Transformation &t, const LocalEquatorialCoordinates& a)
{
// the Transformation is ORTHOGONAL, T^-1 = T'
	double c1 = cos(a.dec * DEGREE), c2 = cos(a.ha * DEGREE);
	double s1 = sin(a.dec * DEGREE), s2 = sin(a.ha * DEGREE);
	Transformation tp = t;
	tp.transpose();
	CartesianVector X = CartesianVector(c1 * c2, -c1 * s2, s1) * tp;

	return LocalEquatorialCoordinates(asin(clamp(X.z)) * RADIAN, atan2(-X.y, X.x) * RADIAN);
}

LocalEquatorialCoordinates CelestialMath::applyConeError(const LocalEquatorialCoordinates& a, double cone)
{
	return LocalEquatorialCoordinates(asin(clamp(sin(a.dec * DEGREE) / cos(cone * DEGREE))) * RADIAN, a.ha - asin(clamp(tan(a.dec * DEGREE) * tan(cone * DEGREE))) * RADIAN);
}

LocalEquatorialCoordinates CelestialMath::deapplyConeError(const LocalEquatorialCoordinates& a, double cone)
{
	double lmd = asin(clamp(sin(a.dec * DEGREE) * cos(cone * DEGREE))) * RADIAN;
	if (lmd > 90 - eps || lmd < -90 + eps) // This implies cone=0, so we don't do anything
		return a;
	double phi = a.ha + asin(clamp(tan(cone * DEGREE) * tan(lmd * DEGREE))) * RADIAN;
	return LocalEquatorialCoordinates(lmd, phi);
}

MountCoordinates CelestialMath::localEquatorialToMount(const LocalEquatorialCoordinates& a, pierside_t side)
{
	MountCoordinates m;
	double ha = a.ha;
	if (side == PIER_SIDE_WEST || (side == PIER_SIDE_AUTO && (ha = remainder(a.ha, 360.0)) > 0))
	{
		m.side = PIER_SIDE_WEST;
		m.dec_delta = 90.0 - a.dec; // dec_delta > 0
		m.ra_delta = ha - 90.0;
	}
	else
	{
		m.side = PIER_SIDE_EAST;
		m.dec_delta = a.dec - 90; // dec_delta<0
		m.ra_delta = ha + 90.0;
	}
	return m;
}

LocalEquatorialCoordinates CelestialMath::mountToLocalEquatorial(const MountCoordinates& m)
{
	LocalEquatorialCoordinates a;
	if (m.side == PIER_SIDE_WEST)
	{
		a.ha = m.ra_delta + 90.0;
		a.dec = 90.0 - m.dec_delta;
	}
	else
	{
		a.ha = m.ra_delta - 90;
		a.dec = 90.0 + m.dec_delta;
	}
	return a;
}

AzimuthalCoordinates CelestialMath::alignOneStar(const LocalEquatorialCoordinates &star_ref, const LocalEquatorialCoordinates &star_meas, const LocationCoordinates& loc,
		const AzimuthalCoordinates &pa_start)
{
	AzimuthalCoordinates pa = pa_start;

// Perform Newton iteration to obtain a better estimation for PA coordinates

	int i = 0;
	double diff = 1e10;
	Transformation t, t1, t2;
	bool diverge = false;

	while (i++ <= MAX_ITERATION && diff > tol)
	{
		getMisalignedPolarAxisTransformation(t, pa, loc);
		getMisalignedPolarAxisTransformation(t1, AzimuthalCoordinates(pa.alt + delta, pa.azi), loc);
		getMisalignedPolarAxisTransformation(t2, AzimuthalCoordinates(pa.alt, pa.azi + delta), loc);

		LocalEquatorialCoordinates star = applyMisalignment(t, star_ref), star1 = applyMisalignment(t1, star_ref), star2 = applyMisalignment(t2, star_ref);

		// Calculate Jacobian matrix. Everything should be divided by delta
		double j11 = star1.dec - star.dec, j12 = star2.dec - star.dec;
		double j21 = star1.ha - star.ha, j22 = star2.ha - star.ha;
		double det = j11 * j22 - j12 * j21;

		if (det == 0)
		{
			diverge = true;
			break;
		}

		// Newton's Method
		// Everything should be multiplied by delta
		double dp1 = -(j22 * (star.dec - star_meas.dec) - j12 * (star.ha - star_meas.ha)) / det;
		double dp2 = -(-j21 * (star.dec - star_meas.dec) + j11 * (star.ha - star_meas.ha)) / det;

		// Update the coordinates
		pa.alt += dp1 * delta;
		pa.azi += dp2 * delta;

		diff = sqrt(dp1 * dp1 + dp2 * dp2) * delta; // calculate the difference
		debug_if(CM_DEBUG, "Iteration %i, %f\t%f\tdiff=%f\t %e, %e, %e, %e\n", i, pa.alt, pa.azi, diff, j11, j12, j21, j22);
	}
	if (diverge)
	{
		/// Do something
		debug_if(CM_DEBUG, "Diverge\n");
	}
	debug_if(CM_DEBUG, "Final delta: %.2e\n", diff);
	return pa;
}

IndexOffset CelestialMath::alignOneStarForOffset(const LocalEquatorialCoordinates& star_ref, const MountCoordinates& star_meas)
{
// Convert the reference star to Mount coordinates using the same pier side setting
	MountCoordinates star = localEquatorialToMount(star_ref, star_meas.side);
	return IndexOffset(star_meas.dec_delta - star.dec_delta, star_meas.ra_delta - star.ra_delta);
}

void CelestialMath::alignTwoStars(const LocalEquatorialCoordinates star_ref[], const LocalEquatorialCoordinates star_meas[], const LocationCoordinates& loc, AzimuthalCoordinates& pa,
		LocalEquatorialCoordinates& offset)
{
	int i = 0;
	double diff = 1e10;
	Transformation t, t1, t2;
	bool diverge = false;

// First try to get a rough estimate for offset to avoid divergence
//	getMisalignedPolarAxisTransformation(t, pa, loc);
//	offset = star_meas[0] - applyMisalignment(t, star_ref[0]);

	while (i++ <= MAX_ITERATION && diff > tol)
	{
		getMisalignedPolarAxisTransformation(t, pa, loc);
		getMisalignedPolarAxisTransformation(t1, AzimuthalCoordinates(pa.alt + delta, pa.azi), loc);
		getMisalignedPolarAxisTransformation(t2, AzimuthalCoordinates(pa.alt, pa.azi + delta), loc);

		// Transform both starts and add offset
		LocalEquatorialCoordinates star[2] =
		{ applyMisalignment(t, star_ref[0]) + offset, applyMisalignment(t, star_ref[1]) + offset };
		LocalEquatorialCoordinates star1[2] =
		{ applyMisalignment(t1, star_ref[0]) + offset, applyMisalignment(t1, star_ref[1]) + offset };
		LocalEquatorialCoordinates star2[2] =
		{ applyMisalignment(t2, star_ref[0]) + offset, applyMisalignment(t2, star_ref[1]) + offset };

		// Calculate Jacobian matrix. Everything should be divided by delta
		// The 4x4 matrix has a special structure, it can be blocked as
		// J1  I
		// J2  I
		// Where J1 and J2 are the 2x2 Jacobians as in alignOneStar and I is 2x2 identity matrix
		// Calculate J1=J
		double j11 = star1[0].dec - star[0].dec, j12 = star2[0].dec - star[0].dec;
		double j21 = star1[0].ha - star[0].ha, j22 = star2[0].ha - star[0].ha;
		// Calculate J2=K
		double k11 = star1[1].dec - star[1].dec, k12 = star2[1].dec - star[1].dec;
		double k21 = star1[1].ha - star[1].ha, k22 = star2[1].ha - star[1].ha;
		double det = (j11 - k11) * (j22 - k22) - (j12 - k12) * (j21 - k21); // det(J) = det(J1-J2)
		if (det == 0)
		{
			diverge = true;
			break;
		}

		// Calculate invert of J1-J2
		double i11 = (j22 - k22) / det, i12 = -(j12 - k12) / det;
		double i21 = -(j21 - k21) / det, i22 = (j11 - k11) / det;
		// Calculate J2(J1-J2)^-1
		double l11 = k11 * i11 + k12 * i21, l12 = k11 * i12 + k12 * i22;
		double l21 = k21 * i11 + k22 * i21, l22 = k21 * i12 + k22 * i22;

		// Calculate F1, F2, F3, F4
		double f1 = star[0].dec - star_meas[0].dec;
		double f2 = star[0].ha - star_meas[0].ha;
		double f3 = star[1].dec - star_meas[1].dec;
		double f4 = star[1].ha - star_meas[1].ha;

		// Newton's Method - Calculate J^-1 * F
		// dp1,2 should be multiplied by delta
		double dp1 = i11 * f1 + i12 * f2 - i11 * f3 - i12 * f4;
		double dp2 = i21 * f1 + i22 * f2 - i21 * f3 - i22 * f4;
		double dp3 = -l11 * f1 - l12 * f2 + (1 + l11) * f3 + l12 * f4;
		double dp4 = -l21 * f1 - l22 * f2 + l21 * f3 + (1 + l22) * f4;

		// Update the coordinates
		pa.alt += -dp1 * delta;
		pa.azi += -dp2 * delta;
		offset.dec += -dp3;
		offset.ha += -dp4;

		diff = sqrt(f1 * f1 + f2 * f2 + f3 * f3 + f4 * f4); // calculate the difference
		debug_if(CM_DEBUG, "Iteration %i, %f\t%f\t%f\t%f\tdiff=%f\t %e %e\n", i, pa.alt, pa.azi, offset.dec, offset.ha, diff, det, det * (i11 * i22 - i12 * i21));
	}
	if (diverge)
	{
		/// Do something
		debug_if(CM_DEBUG, "Diverge\n");
	}
	debug_if(CM_DEBUG, "Final delta: %.2e\n", diff);
}

void CelestialMath::alignTwoStars(const LocalEquatorialCoordinates star_ref[], const MountCoordinates star_meas[], const LocationCoordinates& loc, AzimuthalCoordinates& pa, IndexOffset& offset,
bool &diverge)
{
// Initialize the PA and offset
	pa.alt = loc.lat;
	pa.azi = 0;
	offset = IndexOffset(0, 0);

	int i = 0;
	double residue = 1e10;
	Transformation t, t1, t2;
	diverge = false;

	while (i++ <= MAX_ITERATION && residue > tol)
	{
		getMisalignedPolarAxisTransformation(t, pa, loc);
		getMisalignedPolarAxisTransformation(t1, AzimuthalCoordinates(pa.alt + delta, pa.azi), loc);
		getMisalignedPolarAxisTransformation(t2, AzimuthalCoordinates(pa.alt, pa.azi + delta), loc);

		// Transform both starts and add offset
		MountCoordinates star[2] =
		{ localEquatorialToMount(applyMisalignment(t, star_ref[0]), star_meas[0].side) + offset, localEquatorialToMount(applyMisalignment(t, star_ref[1]), star_meas[1].side) + offset };
		MountCoordinates star1[2] =
		{ localEquatorialToMount(applyMisalignment(t1, star_ref[0]), star_meas[0].side) + offset, localEquatorialToMount(applyMisalignment(t1, star_ref[1]), star_meas[1].side) + offset };
		MountCoordinates star2[2] =
		{ localEquatorialToMount(applyMisalignment(t2, star_ref[0]), star_meas[0].side) + offset, localEquatorialToMount(applyMisalignment(t2, star_ref[1]), star_meas[1].side) + offset };

		// Calculate Jacobian matrix. Everything should be divided by delta
		// The 4x4 matrix has a special structure, it can be blocked as
		// J1  I
		// J2  I
		// Where J1 and J2 are the 2x2 Jacobians as in alignOneStar and I is 2x2 identity matrix
		// Calculate J1=J
		double j11 = star1[0].dec_delta - star[0].dec_delta, j12 = star2[0].dec_delta - star[0].dec_delta;
		double j21 = star1[0].ra_delta - star[0].ra_delta, j22 = star2[0].ra_delta - star[0].ra_delta;
		// Calculate J2=K
		double k11 = star1[1].dec_delta - star[1].dec_delta, k12 = star2[1].dec_delta - star[1].dec_delta;
		double k21 = star1[1].ra_delta - star[1].ra_delta, k22 = star2[1].ra_delta - star[1].ra_delta;
		double det = (j11 - k11) * (j22 - k22) - (j12 - k12) * (j21 - k21); // det(J) = det(J1-J2)
		if (det == 0)
		{
			diverge = true;
			break;
		}

		// Calculate invert of J1-J2
		double i11 = (j22 - k22) / det, i12 = -(j12 - k12) / det;
		double i21 = -(j21 - k21) / det, i22 = (j11 - k11) / det;
		// Calculate J2(J1-J2)^-1
		double l11 = k11 * i11 + k12 * i21, l12 = k11 * i12 + k12 * i22;
		double l21 = k21 * i11 + k22 * i21, l22 = k21 * i12 + k22 * i22;

		// Calculate F1, F2, F3, F4
		double f1 = star[0].dec_delta - star_meas[0].dec_delta;
		double f2 = star[0].ra_delta - star_meas[0].ra_delta;
		double f3 = star[1].dec_delta - star_meas[1].dec_delta;
		double f4 = star[1].ra_delta - star_meas[1].ra_delta;

		// Newton's Method - Calculate J^-1 * F
		// dp1,2 should be multiplied by delta
		double dp1 = i11 * f1 + i12 * f2 - i11 * f3 - i12 * f4;
		double dp2 = i21 * f1 + i22 * f2 - i21 * f3 - i22 * f4;
		double dp3 = -l11 * f1 - l12 * f2 + (1 + l11) * f3 + l12 * f4;
		double dp4 = -l21 * f1 - l22 * f2 + l21 * f3 + (1 + l22) * f4;

		// Update the coordinates
		pa.alt += -dp1 * delta;
		pa.azi += -dp2 * delta;
		offset.dec_off += -dp3;
		offset.ra_off += -dp4;

		residue = sqrt(f1 * f1 + f2 * f2 + f3 * f3 + f4 * f4); // calculate the difference
		debug_if(CM_DEBUG, "Iteration %i, %f\t%f\t%f\t%f\tdiff=%f\t %e %e\n", i, pa.alt, pa.azi, offset.dec_off, offset.ra_off, residue, det, det * (i11 * i22 - i12 * i21));
	}
	if (diverge)
	{
		/// Do something
		debug_if(CM_DEBUG, "Diverge\n");
	}
	debug_if(CM_DEBUG, "Final delta: %.2e\n", residue);
}

static double jac[20][5]; // can maximally hold 10 stars
static double jacjac[5][5]; // J'J
static double invj[5][5];

static void get_corrected_stars(const int N, LocalEquatorialCoordinates stars[], const LocalEquatorialCoordinates star_ref[], const LocationCoordinates& loc, const AzimuthalCoordinates& pa,
		const LocalEquatorialCoordinates& offset, double cone)
{
	static Transformation t;
	CelestialMath::getMisalignedPolarAxisTransformation(t, pa, loc);
	for (int i = 0; i < N; i++)
	{
		stars[i] = CelestialMath::applyConeError(CelestialMath::applyMisalignment(t, star_ref[i]), cone) + offset;
	}
}

static void fill_jacobian(const int N, const int j, LocalEquatorialCoordinates stars0[], LocalEquatorialCoordinates stars1[], const double &dd)
{
	for (int i = 0; i < N; i++)
	{
		jac[i * 2][j] = (stars1[i].dec - stars0[i].dec) / dd;
		jac[i * 2 + 1][j] = (stars1[i].ha - stars0[i].ha) / dd;
	}
}

static double det33(int a1, int a2, int a3, int b1, int b2, int b3)
{
	return jacjac[a1][b1] * jacjac[a2][b2] * jacjac[a3][b3] + jacjac[a1][b2] * jacjac[a2][b3] * jacjac[a3][b1] + jacjac[a1][b3] * jacjac[a2][b1] * jacjac[a3][b2]
			- jacjac[a1][b1] * jacjac[a2][b3] * jacjac[a3][b2] - jacjac[a1][b2] * jacjac[a2][b1] * jacjac[a3][b3] - jacjac[a1][b3] * jacjac[a2][b2] * jacjac[a3][b1];
}

static double det44(int a1, int a2, int a3, int a4, int b1, int b2, int b3, int b4)
{
	return jacjac[a1][b1] * det33(a2, a3, a4, b2, b3, b4) - jacjac[a1][b2] * det33(a2, a3, a4, b1, b3, b4) + jacjac[a1][b3] * det33(a2, a3, a4, b1, b2, b4)
			- jacjac[a1][b4] * det33(a2, a3, a4, b1, b2, b3);
}

static void invert()
{
	invj[0][0] = det44(1, 2, 3, 4, 1, 2, 3, 4);
	invj[1][0] = -det44(1, 2, 3, 4, 0, 2, 3, 4);
	invj[2][0] = det44(1, 2, 3, 4, 0, 1, 3, 4);
	invj[3][0] = -det44(1, 2, 3, 4, 0, 1, 2, 4);
	invj[4][0] = det44(1, 2, 3, 4, 0, 1, 2, 3);

	double det55 = invj[0][0] * jacjac[0][0] + invj[1][0] * jacjac[0][1] + invj[2][0] * jacjac[0][2] + invj[3][0] * jacjac[0][3] + invj[4][0] * jacjac[0][4];
	double idet55 = 1.0 / det55;

	invj[0][0] *= idet55;
	invj[1][0] *= idet55;
	invj[2][0] *= idet55;
	invj[3][0] *= idet55;
	invj[4][0] *= idet55;

	invj[0][1] = -det44(0, 2, 3, 4, 1, 2, 3, 4) * idet55;
	invj[1][1] = det44(0, 2, 3, 4, 0, 2, 3, 4) * idet55;
	invj[2][1] = -det44(0, 2, 3, 4, 0, 1, 3, 4) * idet55;
	invj[3][1] = det44(0, 2, 3, 4, 0, 1, 2, 4) * idet55;
	invj[4][1] = -det44(0, 2, 3, 4, 0, 1, 2, 3) * idet55;

	invj[0][2] = det44(0, 1, 3, 4, 1, 2, 3, 4) * idet55;
	invj[1][2] = -det44(0, 1, 3, 4, 0, 2, 3, 4) * idet55;
	invj[2][2] = det44(0, 1, 3, 4, 0, 1, 3, 4) * idet55;
	invj[3][2] = -det44(0, 1, 3, 4, 0, 1, 2, 4) * idet55;
	invj[4][2] = det44(0, 1, 3, 4, 0, 1, 2, 3) * idet55;

	invj[0][3] = -det44(0, 1, 2, 4, 1, 2, 3, 4) * idet55;
	invj[1][3] = det44(0, 1, 2, 4, 0, 2, 3, 4) * idet55;
	invj[2][3] = -det44(0, 1, 2, 4, 0, 1, 3, 4) * idet55;
	invj[3][3] = det44(0, 1, 2, 4, 0, 1, 2, 4) * idet55;
	invj[4][3] = -det44(0, 1, 2, 4, 0, 1, 2, 3) * idet55;

	invj[0][4] = det44(0, 1, 2, 3, 1, 2, 3, 4) * idet55;
	invj[1][4] = -det44(0, 1, 2, 3, 0, 2, 3, 4) * idet55;
	invj[2][4] = det44(0, 1, 2, 3, 0, 1, 3, 4) * idet55;
	invj[3][4] = -det44(0, 1, 2, 3, 0, 1, 2, 4) * idet55;
	invj[4][4] = det44(0, 1, 2, 3, 0, 1, 2, 3) * idet55;
}

static inline double sqr(double x)
{
	return x * x;
}

void CelestialMath::alignNStars(const int N, const LocalEquatorialCoordinates star_ref[], const LocalEquatorialCoordinates star_meas[], const LocationCoordinates& loc, AzimuthalCoordinates& pa,
		LocalEquatorialCoordinates& offset, double& cone)
{
	if (N == 2)
	{
		alignTwoStars(star_ref, star_meas, loc, pa, offset);
		cone = 0;
		return;
	}
	if (N <= 1)
	{
		return;
	}

// Assuming the cone error is not huge, we should be fairly close to the local minimum
	int i = 0;
	double residue = 1e10;
	LocalEquatorialCoordinates stars0[N], stars1[N];
	double dp[5];
	double f[20];

	while (i++ < MAX_ITERATION_OPTIMIZATION && residue > tol)
	{
		// Calulate Jacobian
		get_corrected_stars(N, stars0, star_ref, loc, pa, offset, cone);
		/*Vary pa.alt*/
		get_corrected_stars(N, stars1, star_ref, loc, AzimuthalCoordinates(pa.alt + delta, pa.azi), offset, cone);
		fill_jacobian(N, 0, stars0, stars1, delta);
		/*Vary pa.azi*/
		get_corrected_stars(N, stars1, star_ref, loc, AzimuthalCoordinates(pa.alt, pa.azi + delta), offset, cone);
		fill_jacobian(N, 1, stars0, stars1, delta);
		/*Vary offset.dec*/
		get_corrected_stars(N, stars1, star_ref, loc, pa, LocalEquatorialCoordinates(offset.dec + delta, offset.ha), cone);
		fill_jacobian(N, 2, stars0, stars1, delta);
		/*Vary offset.ha*/
		get_corrected_stars(N, stars1, star_ref, loc, pa, LocalEquatorialCoordinates(offset.dec, offset.ha + delta), cone);
		fill_jacobian(N, 3, stars0, stars1, delta);
		/*Vary cone*/
		get_corrected_stars(N, stars1, star_ref, loc, pa, offset, cone + delta);
		fill_jacobian(N, 4, stars0, stars1, delta);

		// The Jacobian is now filled. It is 2*N rows and 5 columns
		// Gauss-Newton method: x_n - x_(n-1) = - (J'J)^-1J' * f_(n-1)
		// 1. Matrix multiplication
		int p, q, r;

		for (p = 0; p < 5; p++)
		{
			for (q = 0; q < 5; q++)
			{
				double s = 0;
				for (r = 0; r < 2 * N; r++)
					s += jac[r][p] * jac[r][q];
				jacjac[p][q] = s;
			}
		}

		// 2. Matrix inversion
		invert();

		// 3. Calculate f_(n-1)
		double newresidue = 0;
		for (p = 0; p < N; p++)
		{
			f[2 * p] = stars0[p].dec - star_meas[p].dec;
			f[2 * p + 1] = stars0[p].ha - star_meas[p].ha;
			newresidue += sqr(f[2 * p]) + sqr(f[2 * p + 1]);
		}
		newresidue = sqrt(newresidue);
		// 4. Matrix multiplication
		for (p = 0; p < 5; p++)
		{
			double s = 0;
			for (q = 0; q < 5; q++)
			{
				for (r = 0; r < 2 * N; r++)
				{
					s += invj[p][q] * jac[r][q] * f[r];
				}
			}
			dp[p] = -s;
		}
		// 5. Apply the correction
		pa.alt += dp[0];
		pa.azi += dp[1];
		offset.dec += dp[2];
		offset.ha += dp[3];
		cone += dp[4];

		if (newresidue >= residue - tol)
		{
			debug_if(CM_DEBUG, "Converged.\n");
			break;
		}
		else
		{
			residue = newresidue;
		}
		debug_if(CM_DEBUG, "Iteration %i, %f\t%f\t%f\t%f\t%f\tr=%f\n", i, pa.alt, pa.azi, offset.dec, offset.ha, cone, residue);
	}

	debug_if(CM_DEBUG, "Final result: %f\t%f\t%f\t%f\t%f\tr=%f\n", pa.alt, pa.azi, offset.dec, offset.ha, cone, residue);
}

static void get_corrected_stars(const int N, MountCoordinates stars[], const LocalEquatorialCoordinates star_ref[], const MountCoordinates star_meas[], const LocationCoordinates& loc,
		const AzimuthalCoordinates& pa, const IndexOffset& offset, double cone)
{
	static Transformation t;
	CelestialMath::getMisalignedPolarAxisTransformation(t, pa, loc);
	for (int i = 0; i < N; i++)
	{
		// Misalign, apply cone error, and transform to mount coordinates using the same pier side as in the measured stars
		stars[i] = CelestialMath::localEquatorialToMount(CelestialMath::applyConeError(CelestialMath::applyMisalignment(t, star_ref[i]), cone), star_meas[i].side) + offset;
	}
}

static void fill_jacobian(const int N, const int j, MountCoordinates stars0[], MountCoordinates stars1[], const double &dd)
{
	for (int i = 0; i < N; i++)
	{
		jac[i * 2][j] = (stars1[i].dec_delta - stars0[i].dec_delta) / dd;
		jac[i * 2 + 1][j] = (stars1[i].ra_delta - stars0[i].ra_delta) / dd;
	}
}

void CelestialMath::alignNStars(const int N, const LocalEquatorialCoordinates star_ref[], const MountCoordinates star_meas[], const LocationCoordinates& loc, AzimuthalCoordinates& pa,
		IndexOffset& offset, double& cone,
		bool &diverge)
{
	if (N == 2)
	{
		alignTwoStars(star_ref, star_meas, loc, pa, offset, diverge);
		cone = 0;
		return;
	}
	if (N <= 1)
	{
		return;
	}

// Assuming the cone error is not huge, we should be fairly close to the local minimum
	int i = 0;
	double residue = 1e10;
	MountCoordinates stars0[N], stars1[N];
	double dp[5];
	double f[20];

	diverge = true;

	while (i++ < MAX_ITERATION_OPTIMIZATION && residue > tol)
	{
		// Calulate Jacobian
		get_corrected_stars(N, stars0, star_ref, star_meas, loc, pa, offset, cone);
		/*Vary pa.alt*/
		get_corrected_stars(N, stars1, star_ref, star_meas, loc, AzimuthalCoordinates(pa.alt + delta, pa.azi), offset, cone);
		fill_jacobian(N, 0, stars0, stars1, delta);
		/*Vary pa.azi*/
		get_corrected_stars(N, stars1, star_ref, star_meas, loc, AzimuthalCoordinates(pa.alt, pa.azi + delta), offset, cone);
		fill_jacobian(N, 1, stars0, stars1, delta);
		/*Vary offset.dec*/
		get_corrected_stars(N, stars1, star_ref, star_meas, loc, pa, IndexOffset(offset.dec_off + delta, offset.ra_off), cone);
		fill_jacobian(N, 2, stars0, stars1, delta);
		/*Vary offset.ha*/
		get_corrected_stars(N, stars1, star_ref, star_meas, loc, pa, IndexOffset(offset.dec_off, offset.ra_off + delta), cone);
		fill_jacobian(N, 3, stars0, stars1, delta);
		/*Vary cone*/
		get_corrected_stars(N, stars1, star_ref, star_meas, loc, pa, offset, cone + delta);
		fill_jacobian(N, 4, stars0, stars1, delta);

		// The Jacobian is now filled. It is 2*N rows and 5 columns
		// Gauss-Newton method: x_n - x_(n-1) = - (J'J)^-1J' * f_(n-1)
		// 1. Matrix multiplication
		int p, q, r;

		for (p = 0; p < 5; p++)
		{
			for (q = 0; q < 5; q++)
			{
				double s = 0;
				for (r = 0; r < 2 * N; r++)
					s += jac[r][p] * jac[r][q];
				jacjac[p][q] = s;
			}
		}

		// 2. Matrix inversion
		invert();

		// 3. Calculate f_(n-1)
		double newresidue = 0;
		for (p = 0; p < N; p++)
		{
			f[2 * p] = stars0[p].dec_delta - star_meas[p].dec_delta;
			f[2 * p + 1] = stars0[p].ra_delta - star_meas[p].ra_delta;
			newresidue += sqr(f[2 * p]) + sqr(f[2 * p + 1]);
		}
		newresidue = sqrt(newresidue);
		// 4. Matrix multiplication
		for (p = 0; p < 5; p++)
		{
			double s = 0;
			for (q = 0; q < 5; q++)
			{
				for (r = 0; r < 2 * N; r++)
				{
					s += invj[p][q] * jac[r][q] * f[r];
				}
			}
			dp[p] = -s;
		}
		// 5. Apply the correction
		pa.alt += dp[0];
		pa.azi += dp[1];
		offset.dec_off += dp[2];
		offset.ra_off += dp[3];
		cone += dp[4];

		if (newresidue >= residue - tol)
		{
			debug_if(CM_DEBUG, "Converged.\n");
			diverge = false;
			break;
		}
		else
		{
			residue = newresidue;
		}
		debug_if(CM_DEBUG, "Iteration %i, %f\t%f\t%f\t%f\t%f\tr=%f\n", i, pa.alt, pa.azi, offset.dec_off, offset.ra_off, cone, residue);
	}

	if (diverge)
	{
		debug_if(CM_DEBUG, "Diverged.\n");
	}

	debug_if(CM_DEBUG, "Final result: %f\t%f\t%f\t%f\t%f\tr=%f\n", pa.alt, pa.azi, offset.dec_off, offset.ra_off, cone, residue);

}

EqCalibration CelestialMath::align(const int N, const AlignmentStar stars[], const LocationCoordinates &loc, bool &diverge)
{
	EqCalibration calib;
	calib.pa.alt = loc.lat;
	if (N == 1)
	{
		calib.offset = alignOneStarForOffset(stars[0].star_ref_local(loc), stars[0].star_meas);
		diverge = false;
	}
	else
	{
		LocalEquatorialCoordinates star_ref[N];
		MountCoordinates star_meas[N];
		for (int i = 0; i < N; i++)
		{
			star_ref[i] = stars[i].star_ref_local(loc);
			star_meas[i] = stars[i].star_meas;
		}
		if (N == 2)
		{
			alignTwoStars(star_ref, star_meas, loc, calib.pa, calib.offset, diverge);
		}
		else
		{
			alignNStars(N, star_ref, star_meas, loc, calib.pa, calib.offset, calib.cone, diverge);
		}
	}

	return calib;
}

double CelestialMath::parseHMSAngle(char* hms)
{
	char *h = strchr(hms, 'h');
	char *m = strchr(hms, 'm');
	char *s = strchr(hms, 's');
	if (h == NULL || m == NULL || s == NULL || !(h < m && m < s))
	{
		return NAN;
	}

	*h = '\0';
	*m = '\0';
	*s = '\0';

	char *tp;
	int hour = strtol(hms, &tp, 10);
	if (tp == hms)
	{
		return NAN;
	}
	int minute = strtol(h + 1, &tp, 10);
	if (tp == h + 1)
	{
		return NAN;
	}
	double second = strtod(m + 1, &tp);
	if (tp == m + 1)
	{
		return NAN;
	}

	if (!(hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 60))
	{
		return NAN;
	}

	return remainder((hour + minute / 60.0 + second / 3600.0) * 15.0, 360.0);
}

double CelestialMath::parseDMSAngle(char* dms)
{
	char *d = strchr(dms, 'd');
	char *m = strchr(dms, 'm');
	char *s = strchr(dms, 's');
	if (d == NULL || m == NULL || s == NULL || !(d < m && m < s))
	{
		return NAN;
	}

	*d = '\0';
	*m = '\0';
	*s = '\0';

	char *tp;
	int degree = strtol(dms, &tp, 10);
	if (tp == dms)
	{
		return NAN;
	}
	int arcminute = strtol(d + 1, &tp, 10);
	if (tp == d + 1)
	{
		return NAN;
	}
	double arcsecond = strtod(m + 1, &tp);
	if (tp == m + 1)
	{
		return NAN;
	}

	if (!(degree >= -180.0 && degree <= 180.0 && arcminute >= 0 && arcminute <= 59 && arcsecond >= 0 && arcsecond <= 60))
	{
		return NAN;
	}

	return remainder((degree + arcminute / 60.0 + arcsecond / 3600.0), 360);
}

EquatorialCoordinates EquatorialCoordinates::precessFromJ2000(time_t timestamp)
{
	double JD = (double) timestamp * 1.1574074074074E-5 + 2440587.5 - 2451545.0; // Julian date since J2000
	double Y = JD / 365.25; // Year since J2000
	// RA/DEC at date
	double RA = ra + (3.075 + 1.336 * sin(ra * DEGREE) * tan(dec * DEGREE)) * Y / 240.0;
	double DEC = dec + 20.04 * cos(ra * DEGREE) * Y / 3600.0;
	return EquatorialCoordinates(DEC, RA);
}

EquatorialCoordinates EquatorialCoordinates::precessToJ2000(time_t timestamp)
{
	double JD = (double) timestamp * 1.1574074074074E-5 + 2440587.5 - 2451545.0; // Julian date since J2000
	double Y = JD / 365.25; // Year since J2000
	// RA/DEC at J2000
	double RA = ra - (3.075 + 1.336 * sin(ra * DEGREE) * tan(dec * DEGREE)) * Y / 240.0;
	double DEC = dec - 20.04 * cos(ra * DEGREE) * Y / 3600.0;
	return EquatorialCoordinates(DEC, RA);
}

double CelestialMath::kingRate(const EquatorialCoordinates &eq, const LocationCoordinates &loc, time_t time)
{
	LocalEquatorialCoordinates leq = CelestialMath::equatorialToLocalEquatorial(eq, time, loc);
//	AzimuthalCoordinates ac = CelestialMath::localEquatorialToAzimuthal(leq,
//			loc);
	double cosLat = cos(loc.lat * DEGREE);
	double sinLat = sin(loc.lat * DEGREE);
	double cotLat = cosLat / sinLat;
	double cosDec = cos(eq.dec * DEGREE);
	double sinDec = sin(eq.dec * DEGREE);
	double tanDec = sinDec / cosDec;
	double cosHA = cos(leq.ha * DEGREE);
	double kingMpD = (1436.46 + 0.4 * (cosLat / cosDec * (cosLat * cosDec + sinLat * sinDec * cosHA) / pow(sinLat * sinDec + cosLat * cosDec * cosHA, 2.0) - cotLat * tanDec * cosHA));
	return 6.0 / kingMpD;
}
