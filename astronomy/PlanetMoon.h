/*
 * PlanetMoon.h
 *
 *  Created on: 2018Äê5ÔÂ5ÈÕ
 *      Author: caoyuan9642
 */

#ifndef ASTRONOMY_PLANETMOON_H_
#define ASTRONOMY_PLANETMOON_H_

#include "CelestialMath.h"

struct EquatorialXYZ;

struct EquatorialCoordinatesWithDist: EquatorialCoordinates
{
	double dist; // Distance, in whatever unit you like
	EquatorialCoordinatesWithDist(double dec=0, double ra=0, double dist=0) :
			EquatorialCoordinates(dec, ra), dist(dist)
	{
	}
	EquatorialCoordinatesWithDist(EquatorialCoordinates eq, double dist) :
			EquatorialCoordinates(eq), dist(dist)
	{
	}
	operator EquatorialCoordinates() const // Overloaded conversion
	{
		return EquatorialCoordinates(dec, ra);
	}

	/**
	 * Precess from and to J2000 using date specified by the timestamp
	 */
	EquatorialCoordinatesWithDist precessFromJ2000(time_t timestamp) const
	{
		return EquatorialCoordinatesWithDist(EquatorialCoordinates(dec, ra).precessFromJ2000(timestamp), dist);
	}
	EquatorialCoordinatesWithDist precessToJ2000(time_t timestamp) const
	{
		return EquatorialCoordinatesWithDist(EquatorialCoordinates(dec, ra).precessToJ2000(timestamp), dist);
	}

	EquatorialXYZ toXYZ() const;
};

struct MoonPhase{
	double illumangle;
	double illumaxis;
};

struct EquatorialXYZ
{
	double x;
	double y;
	double z;
	EquatorialXYZ(double x = 0, double y = 0, double z = 0) :
			x(x), y(y), z(z)
	{
	}
	EquatorialXYZ operator-() const
	{
		return EquatorialXYZ(-x, -y, -z);
	}
	EquatorialXYZ operator-(const EquatorialXYZ & b) const
	{
		return EquatorialXYZ(x - b.x, y - b.y, z - b.z);
	}
	EquatorialCoordinatesWithDist toSpherical() const;
};

class PlanetMoon
{
public:
	typedef enum
	{
		MERCURY = 0, VENUS = 1, MARS = 2, JUPITER = 3, SATURN = 4, URANUS = 5, NEPTUNE = 6, PLUTO = 7,

		SUN = 8, MOON = 9, EARTH = 15,
	} Object;

	static EquatorialCoordinatesWithDist calculatePosition(Object obj, time_t timestamp, LocationCoordinates loc = LocationCoordinates(0, 0));
	static EquatorialCoordinatesWithDist calculateLunarPositionJNow(time_t timestamp, LocationCoordinates loc);
	static MoonPhase getLunarPhase(time_t timestamp, const LocationCoordinates &loc, const EquatorialCoordinatesWithDist &moon);

	static const char *getName(Object obj)
	{
		switch (obj)
		{
		case MERCURY:
			return "Mercury";
		case VENUS:
			return "Venus";
		case MARS:
			return "Mars";
		case JUPITER:
			return "Jupiter";
		case SATURN:
			return "Saturn";
		case URANUS:
			return "Uranus";
		case NEPTUNE:
			return "Neptune";
		case PLUTO:
			return "Pluto";
		case SUN:
			return "Sun";
		case MOON:
			return "Moon";
		case EARTH:
			return "Earth";
		default:
			return "";
		}
	}

	/**
	 * Get a representative color index for the planet
	 */
	static float getColor(Object obj)
	{
		switch (obj)
		{
		case MERCURY:
			return 0.00;
		case VENUS:
			return 0.50;
		case MARS:
			return 3.00;
		case JUPITER:
			return 1.00;
		case SATURN:
			return 0.80;
		case URANUS:
			return -1.00;
		case NEPTUNE:
			return -0.60;
		case PLUTO:
			return 0.00;
		case SUN:
			return 0.30;
		case MOON:
			return 0.00;
		case EARTH:
			return -5.00;
		default:
			return 0.00;
		}
	}

	/**
	 * Get a representative magnitude for the planet
	 */
	static float getMagnitude(Object obj)
	{
		switch (obj)
		{
		case MERCURY:
			return 0.00;
		case VENUS:
			return -4.00;
		case MARS:
			return -1.00;
		case JUPITER:
			return -2.51;
		case SATURN:
			return 0.3;
		case URANUS:
			return 3.00;
		case NEPTUNE:
			return 6.00;
		case PLUTO:
			return 8.00;
		case SUN:
			return -26.00;
		case MOON:
			return -11.00;
		case EARTH:
			return 0.00;
		default:
			return 0.00;
		}
	}

private:
	PlanetMoon();
	virtual ~PlanetMoon()
	{
		// TODO Auto-generated destructor stub
	}

	static EquatorialXYZ calculatePlanetPosition(Object obj, time_t timestamp);
};

#endif /* ASTRONOMY_PLANETMOON_H_ */
