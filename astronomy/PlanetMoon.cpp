/*
 * PlanetMoon.cpp
 *
 *  Created on: 2018Äê5ÔÂ5ÈÕ
 *      Author: caoyuan9642
 */

#include <PlanetMoon.h>
#include <cmath>

static const double DEGREE = M_PI / 180.0;
static const double RADIAN = 180.0 / M_PI;

static const double kepler[9][6] =
{
//			a(au)         e         I(deg)      L(deg)      wb(deg)     Omega(deg)
		{ 0.38709927, 0.20563593, 7.00497902, 252.25032350, 77.45779628, 48.33076593 }, // Mercury
		{ 0.72333566, 0.00677672, 3.39467605, 181.97909950, 131.60246718, 76.67984255 }, // Venus
		{ 1.52371034, 0.09339410, 1.84969142, -4.55343205, -23.94362959, 49.55953891 }, // Mars
		{ 5.20288700, 0.04838624, 1.30439695, 34.39644051, 14.72847983, 100.47390909 }, // Jupiter
		{ 9.53667594, 0.05386179, 2.48599187, 49.95424423, 92.59887831, 113.66242448 }, // Saturn
		{ 19.18916464, 0.04725744, 0.77263783, 313.23810451, 170.95427630, 74.01692503 }, // Uranus
		{ 30.06992276, 0.00859048, 1.77004347, -55.12002969, 44.96476227, 131.78422574 }, // Neptune
		{ 39.48211675, 0.24882730, 17.14001206, 238.92903833, 224.06891629, 110.30393684 }, // Pluto

		{ 1.00000261, 0.01671123, -0.00001531, 100.46457166, 102.93768193, 0.0 }, // Earth
		};

static const double kepler_dt[9][6] =
{
//dt of: 	a(au)         e         I(deg)      L(deg)      wb(deg)     Omega(deg)
		{ 0.00000037, 0.00001906, -0.00594749, 149472.67411175, 0.16047689, -0.12534081 }, // Mercury
		{ 0.00000390, -0.00004107, -0.00078890, 58517.81538729, 0.00268329, -0.27769418 }, // Venus
		{ 0.00001847, 0.00007882, -0.00813131, 19140.30268499, 0.44441088, -0.29257343 }, // Mars
		{ -0.00011607, -0.00013253, -0.00183714, 3034.74612775, 0.21252668, 0.20469106 }, // Jupiter
		{ -0.00125060, -0.00050991, 0.00193609, 1222.49362201, -0.41897216, -0.28867794 }, // Saturn
		{ -0.00196176, -0.00004397, -0.00242939, 428.48202785, 0.40805281, 0.04240589 }, // Uranus
		{ 0.00026291, 0.00005105, 0.00035372, 218.45945325, -0.32241464, -0.00508664 }, // Neptune
		{ -0.00031596, 0.00005170, 0.00004818, 145.20780515, -0.04062942, -0.01183482 }, // Pluto

		{ 0.00000562, -0.00004392, -0.01294668, 35999.37244981, 0.32327364, 0.0 }, // Earth
		};

EquatorialCoordinatesWithDist PlanetMoon::calculatePosition(Object obj, time_t timestamp, LocationCoordinates loc)
{
	if (obj == MOON)
	{
		// Precess back to J2000
		// See https://www.cv.nrao.edu/~rfisher/Ephemerides/earth_rot.html
		return calculateLunarPositionJNow(timestamp, loc).precessToJ2000(timestamp);
	}
	EquatorialXYZ earth = calculatePlanetPosition(EARTH, timestamp);
	if (obj == SUN)
	{
		// By Definition
		return (-earth).toSpherical();
	}
	else
	{
		EquatorialXYZ planet = calculatePlanetPosition(obj, timestamp);
		return (planet - earth).toSpherical();
	}
}

PlanetMoon::PlanetMoon()
{
}

EquatorialXYZ PlanetMoon::calculatePlanetPosition(Object obj, time_t timestamp)
{
	// See https://ssd.jpl.nasa.gov/?planet_pos
	EquatorialXYZ eq;
	int index = (int) obj;
	if (index < 0 || index > 7)
		index = 8; // Earth
	double JD = (double) timestamp * 1.1574074074074E-5 + 2440587.5 - 2451545.0; // Julian date since J2000
	double T = JD / 36525.0; // Century since J2000

	double a = kepler[index][0] + kepler_dt[index][0] * T;
	double e = kepler[index][1] + kepler_dt[index][1] * T;
	double I = (kepler[index][2] + kepler_dt[index][2] * T) * DEGREE;
	double L = (kepler[index][3] + kepler_dt[index][3] * T) * DEGREE;
	double wb = (kepler[index][4] + kepler_dt[index][4] * T) * DEGREE;
	double Omega = (kepler[index][5] + kepler_dt[index][5] * T) * DEGREE;

	// Angles in RADIAN now

	double w = wb - Omega;
	double M = L - wb;

	// Solve M = E - e sin(E),
	double E = M + e * sin(M);
	double deltaE = 1e10;

	while (deltaE > 1e-6 * DEGREE)
	{
		double deltaM = M - (E - e * sin(E));

		deltaE = deltaM / (1 - e * cos(E));
		E += deltaE;
	}

	double xp = a * (cos(E) - e);
	double yp = a * sqrt(1 - e * e) * sin(E);

	// Heliocentric coordinates
	double xe = (cos(w) * cos(Omega) - sin(w) * sin(Omega) * cos(I)) * xp + (-sin(w) * cos(Omega) - cos(w) * sin(Omega) * cos(I)) * yp;
	double ye = (cos(w) * sin(Omega) + sin(w) * cos(Omega) * cos(I)) * xp + (-sin(w) * sin(Omega) + cos(w) * cos(Omega) * cos(I)) * yp;
	double ze = (sin(w) * sin(I)) * xp + (cos(w) * sin(I)) * yp;

	// Equatorial coordinates
	const double ecl = 23.43928 * DEGREE; // obliquity of earth at J2000
	eq.x = xe;
	eq.y = cos(ecl) * ye - sin(ecl) * ze;
	eq.z = sin(ecl) * ye + cos(ecl) * ze;

	return eq;
}

EquatorialCoordinatesWithDist EquatorialXYZ::toSpherical() const
{
	double dist = sqrt(x * x + y * y + z * z);
	return EquatorialCoordinatesWithDist(asin(z / dist) * RADIAN, atan2(y, x) * RADIAN, dist);
}

EquatorialXYZ EquatorialCoordinatesWithDist::toXYZ() const
{
	return EquatorialXYZ(dist * cos(dec * DEGREE) * cos(ra * DEGREE), dist * cos(dec * DEGREE) * sin(ra * DEGREE), dist * sin(dec * DEGREE));
}

static inline double check(double x, double limit)
{
	if (fabs(x) < limit)
		return limit;
	else
		return x;
}

EquatorialCoordinatesWithDist PlanetMoon::calculateLunarPositionJNow(time_t timestamp, LocationCoordinates loc)
{
	// See http://stjarnhimlen.se/comp/tutorial.html
	double D = (double) timestamp * 1.1574074074074E-5 + 2440587.5 - 2451543.5; // Date since Dec 31st 00:00:00 2000
	// Moon elements
	double Nm = remainder(125.1228 - 0.0529538083 * D, 360.0) * DEGREE;
	double im = 5.1454 * DEGREE;
	double wm = remainder(318.0634 + 0.1643573223 * D, 360.0) * DEGREE;
	double am = 60.2666; // Earth radii
	double ecm = 0.0549;
	double Mm = remainder(115.3654 + 13.0649929509 * D, 360.0) * DEGREE;
	// Sun elements
//	double Ns = 0;
//	double isun = 0;
	double ws = remainder(282.9404 + 4.70935e-5 * D, 360.0) * DEGREE;
//	double asun = 1; // AU
//	double ecs = 0.016709 - 1.151e-9 * D;
	double Ms = remainder(356.047 + 0.9856002585 * D, 360.0) * DEGREE;
	// Position of moon
	double Em = Mm + ecm * sin(Mm) * (1 + ecm * cos(Mm));
	double deltaE = 1e10;
	// Newton's iteration
	while (deltaE > 1e-6 * DEGREE)
	{
		deltaE = (Mm - (Em - ecm * sin(Em))) / (1 - ecm * cos(Em));
		Em += deltaE;
	}
	double xv = am * (cos(Em) - ecm);
	double yv = am * sqrt(1 - ecm * ecm) * sin(Em);
	double vm = atan2(yv, xv);
	double rm = sqrt(xv * xv + yv * yv);
	double xh = rm * (cos(Nm) * cos(vm + wm) - sin(Nm) * sin(vm + wm) * cos(im));
	double yh = rm * (sin(Nm) * cos(vm + wm) + cos(Nm) * sin(vm + wm) * cos(im));
	double zh = rm * sin(vm + wm) * sin(im);
	double lon = atan2(yh, xh);
	double lat = atan2(zh, sqrt(xh * xh + yh * yh));
	// Perturbations
	double Ls = Ms + ws;
	double Lm = Mm + wm + Nm;
	double dm = Lm - Ls;
	double F = Lm - Nm;
	double dlon = 0;
	dlon -= 1.274 * sin(Mm - 2 * dm);
	dlon += 0.658 * sin(2 * dm);
	dlon -= 0.186 * sin(Ms);
	dlon -= 0.059 * sin(2 * Mm - 2 * dm);
	dlon -= 0.057 * sin(Mm - 2 * dm + Ms);
	dlon += 0.053 * sin(Mm + 2 * dm);
	dlon += 0.046 * sin(2 * dm - Ms);
	dlon += 0.041 * sin(Mm - Ms);
	dlon -= 0.035 * sin(dm);
	dlon -= 0.031 * sin(Mm + Ms);
	dlon -= 0.015 * sin(2 * F - 2 * dm);
	dlon += 0.011 * sin(Mm - 4 * dm);
	lon += dlon * DEGREE;
	double dlat = 0;
	dlat -= 0.173 * sin(F - 2 * dm);
	dlat -= 0.055 * sin(Mm - F - 2 * dm);
	dlat -= 0.046 * sin(Mm + F - 2 * dm);
	dlat += 0.033 * sin(F + 2 * dm);
	dlat += 0.017 * sin(2 * Mm + F);
	lat += dlat * DEGREE;

	rm -= 0.58 * cos(Mm - 2 * dm);
	rm -= 0.46 * cos(2 * dm);

	double xg = rm * cos(lon) * cos(lat);
	double yg = rm * sin(lon) * cos(lat);
	double zg = rm * sin(lat);

	double ecl = (23.4393 - 3.563e-7 * D) * DEGREE; // obliquity at date
//	double ecl = (23.43928) * DEGREE; // obliquity at J2000
	EquatorialXYZ eq;
	eq.x = xg;
	eq.y = yg * cos(ecl) - zg * sin(ecl);
	eq.z = yg * sin(ecl) + zg * cos(ecl);

	EquatorialCoordinates eqr = eq.toSpherical();

	// Parallax correction
	double mpar = asin(1 / rm) * RADIAN; // Parallax angle in degrees
	double gclat = loc.lat - 0.1924 * sin(2 * loc.lat * DEGREE); // Geometrical latitude at location
	double rho = 0.99833 + 0.00167 * cos(2 * loc.lat * DEGREE); // Earth radius at location
	double ha = CelestialMath::getLocalSiderealTime(timestamp, loc) - eqr.ra; // Hour angle in degrees

	double g = atan2(tan(gclat * DEGREE), cos(ha * DEGREE)) * RADIAN; // aux angle in degrees

	double tRA = eqr.ra - mpar * rho * cos(gclat * DEGREE) * sin(ha * DEGREE) / check(cos(eqr.dec * DEGREE), 1e-15);
	double tDEC;
	if (gclat == 0)
		tDEC = eqr.dec - mpar * rho * sin(-eqr.dec * DEGREE) * cos(ha * DEGREE); // Limit of gclat->0
	else
		tDEC = eqr.dec - mpar * rho * sin(gclat * DEGREE) * sin((g - eqr.dec) * DEGREE) / sin(g * DEGREE);

	return EquatorialCoordinatesWithDist(tDEC, tRA, rm * 12742.0 / 149597871.0); // Distance is in AU
}

MoonPhase PlanetMoon::getLunarPhase(time_t timestamp, const LocationCoordinates &loc, const EquatorialCoordinatesWithDist& moon)
{
	MoonPhase p;
	EquatorialCoordinatesWithDist sun = PlanetMoon::calculatePosition(SUN, timestamp, loc);
//	double JD = (double) timestamp * 1.1574074074074E-5 + 2440587.5 - 2451545.0; // Days since J2000
	EquatorialXYZ s = sun.toXYZ();
	EquatorialXYZ m = moon.toXYZ();

	// Cross product of s and m
	double x = s.y * m.z - s.z * m.y;
	double y = s.z * m.x - s.x * m.z;
	double z = s.x * m.y - s.y * m.x;
//	double r = sqrt(x * x + y * y + z * z);

	// Convert to spherical
	double Delta = atan2(z, sqrt(x * x + y * y));
	double Phi = atan2(y, x);

	// Calculate orientation of illumination
	double Y = cos(Delta) * sin(Phi - moon.ra * DEGREE);
	double X = sin(moon.dec * DEGREE) * cos(Delta) * cos(Phi - moon.ra * DEGREE) - cos(moon.dec * DEGREE) * sin(Delta);
	p.illumaxis = atan2(Y, X) * RADIAN + 180.0;

	// Calculate illumination angle. When we set the illumaxis to upward direction, the sun should always be on the RIGHT side of the moon
	// 0 means sun and moon aligned -> newmoon
	// 180 means sun and moon anti-aligned -> fullmoon
	// Dot product between s-m and m, (s-m).m = s.m - m^2
	double dot = s.x * m.x + s.y * m.y + s.z * m.z - moon.dist * moon.dist;
	p.illumangle = acos(dot / moon.dist / sun.dist) * RADIAN;
	return p;
}

