/*
 * CelestialMath.h
 *
 *  Created on: Feb 21, 2018
 *      Author: Yuan
 */

#ifndef CELESTIALMATH_H_
#define CELESTIALMATH_H_

#include <ctime>
#include <cmath>
#include <cstdio>

static const double sidereal_speed = 0.00417807462; /* deg / s */

struct EquatorialCoordinates
{
	double dec;		// Declination
	double ra;		// Right ascension
	EquatorialCoordinates(double d = 0, double r = 0) :
			dec(d), ra(r)
	{
	}
	void print(FILE *f = stdout)
	{
		char we = (ra > 0) ? 'E' : 'W';
		char ns = (dec > 0) ? 'N' : 'S';
		double r = (ra < 0) ? ra + 360.0 : ra;
		double d = fabs(dec);
		fprintf(f, "%dh%dm%.2fs %c, %d %d'%.2f\" %c", int(r / 15), int(fmod(r, 15.0) * 4), fmod(r, 0.25) * 240, we, int(d), int(fmod(d, 1.0) * 60), fmod(d, 1.0 / 60) * 3600, ns);
	}

	/**
	 * Precess from and to J2000 using date specified by the timestamp
	 */
	EquatorialCoordinates precessFromJ2000(time_t timestamp);
	EquatorialCoordinates precessToJ2000(time_t timestamp);
};

struct LocalEquatorialCoordinates
{
	double dec;		// Declination
	double ha;		// Hour angle
	LocalEquatorialCoordinates(double d = 0, double h = 0) :
			dec(d), ha(h)
	{
	}
	LocalEquatorialCoordinates operator+(const LocalEquatorialCoordinates &b) const
	{
		return LocalEquatorialCoordinates(remainder(dec + b.dec, 360), remainder(ha + b.ha, 360));
	}
	LocalEquatorialCoordinates operator-(const LocalEquatorialCoordinates &b) const
	{
		return LocalEquatorialCoordinates(remainder(dec - b.dec, 360), remainder(ha - b.ha, 360));
	}
};

struct AzimuthalCoordinates
{
	double alt;		// Altitude
	double azi;		// Azimuth
	AzimuthalCoordinates(double a1 = 0, double a2 = 0) :
			alt(a1), azi(a2)
	{
	}
};

struct LocationCoordinates
{
	double lat;		// Latitude
	double lon;		// Longtitude
	LocationCoordinates(double l1 = 0, double l2 = 0) :
			lat(l1), lon(l2)
	{
	}
};

struct Transformation;

struct CartesianVector
{
	double x, y, z;
	CartesianVector(double x = 0, double y = 0, double z = 0) :
			x(x), y(y), z(z)
	{
	}
	CartesianVector operator*(const Transformation &t);
};

struct Transformation
{
	double a11, a12, a13;
	double a21, a22, a23;
	double a31, a32, a33;
	CartesianVector operator*(const CartesianVector &vec);
	void transpose()
	{
		double temp = a12;
		a12 = a21;
		a21 = temp;
		temp = a13;
		a13 = a31;
		a31 = temp;
		temp = a32;
		a32 = a23;
		a23 = temp;
	}
};

typedef enum
{
	PIER_SIDE_EAST, PIER_SIDE_WEST, PIER_SIDE_AUTO = 0
} pierside_t;

struct IndexOffset
{
	double dec_off; // Offset of the index position in DEC
	double ra_off;  // Offset of the index position in RA/HA axis
	IndexOffset(double d = 0, double r = 0) :
			dec_off(d), ra_off(r)
	{
	}
};

struct MountCoordinates
{
	double dec_delta; // Displacement from index position in DEC axis
	double ra_delta;  // Displacement from index position in RA/HA axis
	pierside_t side;
	MountCoordinates(double dec = 0, double ra = 0, pierside_t s = PIER_SIDE_EAST) :
			dec_delta(dec), ra_delta(ra), side(s)
	{
	}
	MountCoordinates operator+(const IndexOffset offset)
	{
		return MountCoordinates(remainder(dec_delta + offset.dec_off, 360), remainder(ra_delta + offset.ra_off, 360), side);
	}
	MountCoordinates operator-(const IndexOffset offset)
	{
		return MountCoordinates(remainder(dec_delta - offset.dec_off, 360), remainder(ra_delta - offset.ra_off, 360), side);
	}
};
/**
 * Alignment star object
 */
struct AlignmentStar
{
	EquatorialCoordinates star_ref; /// Reference position of the star in the sky (in current epoch)
	MountCoordinates star_meas;	/// Measured position of the star in mount coordinates
	time_t timestamp;				/// UTC timestamp of the measurement
	AlignmentStar()
	{
		timestamp = 0;
	}
	AlignmentStar(const EquatorialCoordinates & ref, MountCoordinates meas, time_t t) :
			star_ref(ref), star_meas(meas), timestamp(t)
	{
	}
	LocalEquatorialCoordinates star_ref_local(const LocationCoordinates &loc) const;
};

struct EqCalibration
{
	IndexOffset offset;
	AzimuthalCoordinates pa;
	double cone;
	EqCalibration() :
			cone(0)
	{
	}
	EqCalibration(const IndexOffset &off, const AzimuthalCoordinates p, double c) :
			offset(off), pa(p), cone(c)
	{
	}
};
/**
 * Utility functions for doing math on coordinates of the celestial sphere
 */
class CelestialMath
{
public:
	CelestialMath()
	{
	}
	~CelestialMath()
	{
	}

	/*Basic conversion between reference frames*/
	static AzimuthalCoordinates localEquatorialToAzimuthal(const LocalEquatorialCoordinates &a, const LocationCoordinates &loc);
	static LocalEquatorialCoordinates azimuthalToLocalEquatorial(const AzimuthalCoordinates &b, const LocationCoordinates &loc);
	static double getGreenwichMeanSiderealTime(time_t timestamp);
	static double getLocalSiderealTime(time_t timestamp, const LocationCoordinates &loc);
	static LocalEquatorialCoordinates equatorialToLocalEquatorial(const EquatorialCoordinates &e, time_t timestamp, const LocationCoordinates &loc);
	static EquatorialCoordinates localEquatorialToEquatorial(const LocalEquatorialCoordinates &a, time_t timestamp, const LocationCoordinates &loc);

	/*Misalignment correction functions*/
	static Transformation &getMisalignedPolarAxisTransformation(Transformation &t, const AzimuthalCoordinates &mpa, const LocationCoordinates &loc);
	static LocalEquatorialCoordinates applyMisalignment(const Transformation &t, const LocalEquatorialCoordinates &a);
	static LocalEquatorialCoordinates applyConeError(const LocalEquatorialCoordinates &a, double cone);
	static LocalEquatorialCoordinates deapplyMisalignment(const Transformation &t, const LocalEquatorialCoordinates &a);
	static LocalEquatorialCoordinates deapplyConeError(const LocalEquatorialCoordinates &a, double cone);

	/*Convert to and from Mount coordinates*/
	static MountCoordinates localEquatorialToMount(const LocalEquatorialCoordinates &a, pierside_t side = PIER_SIDE_AUTO);
	static LocalEquatorialCoordinates mountToLocalEquatorial(const MountCoordinates &m);

	/*Alignment procedures*/

	/**
	 * One-star alignment (only for testing), to find the PA misalignment
	 */
	static AzimuthalCoordinates alignOneStar(const LocalEquatorialCoordinates &star_ref, const LocalEquatorialCoordinates &star_meas, const LocationCoordinates &loc,
			const AzimuthalCoordinates &pa_start);

	static IndexOffset alignOneStarForOffset(const LocalEquatorialCoordinates &star_ref, const MountCoordinates &star_meas);

	/*static AzimuthalCoordinates alignOneStar(const LocalEquatorialCoordinates &star_ref, const LocalEquatorialCoordinates &star_meas,
	 const LocationCoordinates &loc, const AzimuthalCoordinates &pa_start);*/
	/**
	 * Two-star alignment for finding PA misalignment as well as offset in both axis
	 * @param star_ref Reference stars (array of 2)
	 * @param star_meas Measured stars (array of 2)
	 * @param loc Location
	 * @param pa Initial PA alt-az coordinates. This parameter will be updated with new values
	 * @param offset Initial offset values. This parameter will be updated with new values
	 */
	static void alignTwoStars(const LocalEquatorialCoordinates star_ref[], const LocalEquatorialCoordinates star_meas[], const LocationCoordinates &loc, AzimuthalCoordinates &pa,
			LocalEquatorialCoordinates &offset);
	static void alignTwoStars(const LocalEquatorialCoordinates star_ref[], const MountCoordinates star_meas[], const LocationCoordinates &loc, AzimuthalCoordinates &pa, IndexOffset &offset,
			bool &diverge);

	/**
	 * N-star alignment for finding PA misalignment, offset, and cone error
	 * This function will first call alignTwoStars with the first two stars assuming no cone error, then run an optimization algorithm to minimize the residual error by tweaking all 5 parameters.
	 * @param N number of alignment stars
	 * @param star_ref Reference stars
	 * @param star_meas Measured stars
	 * @param loc Location
	 * @param pa Initial PA alt-az coordinates. This parameter will be updated with new values
	 * @param offset Initial offset values. This parameter will be updated with new values
	 * @param cone Initial cone error. This parameter will be updated with new values
	 */
	static void alignNStars(const int N, const LocalEquatorialCoordinates star_ref[], const LocalEquatorialCoordinates star_meas[], const LocationCoordinates &loc, AzimuthalCoordinates &pa,
			LocalEquatorialCoordinates &offset, double &cone);
	static void alignNStars(const int N, const LocalEquatorialCoordinates star_ref[], const MountCoordinates star_meas[], const LocationCoordinates &loc, AzimuthalCoordinates &pa, IndexOffset &offset,
			double &cone, bool &diverge);

	/**
	 * Adaptor for EqMount
	 */
	static EqCalibration align(const int N, const AlignmentStar stars[], const LocationCoordinates &loc, bool &diverge);

	/*Utility functions*/

	/*
	 * Convert HMS notation such as 21h54m31.6s to degrees (counting from 0h0m0s = 0degree, from -180 ~ 180 degrees)
	 */
	static double parseHMSAngle(char *hms);

	static double parseDMSAngle(char *dms);

	/**
	 * Calculate King tracking rate based on the star position and location
	 */
	static double kingRate(const EquatorialCoordinates &eq, const LocationCoordinates &loc, time_t time);

};

#endif /* CELESTIALMATH_H_ */
