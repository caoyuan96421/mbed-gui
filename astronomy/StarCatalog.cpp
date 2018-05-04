/*
 * StarCatalog.cpp
 *
 *  Created on: 2018Äê5ÔÂ2ÈÕ
 *      Author: caoyuan9642
 */

#include <StarCatalog.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cctype>

#define MAX_STAR 3300

//const char CATALOG_COMMON[] = "/sdcard/hygcommon.csv";
const char CATALOG_CONSTELLATION[] = "/sdcard/hygconstellation.csv";
const char CATALOG_ALL[] = "/sdcard/hygfull.csv";

// Catalog of constellation stars
__attribute__((section (".sdram")))
                 StarItem catalog_constellation[MAX_STAR];

unsigned int size_catalog_constellation;

QuadTree qt_common;
QuadTree qt_constellation;

__attribute__((section (".sdram")))
        static QTNode _nodefactory[MAX_STAR];

static int _nodecount = 0;

QTNode* QTNode::malloc()
{
	if (_nodecount == MAX_STAR)
		return NULL;
	return &_nodefactory[_nodecount++];
}

StarCatalog::StarCatalog()
{
	loadCatalogs();
}

static char *strtok_ch(char *buf, const char delim, char **saveptr)
{
	if (!buf && saveptr && *saveptr)
	{
		buf = *saveptr;
	}
	char *d = strchr(buf, delim);
	if (!d)
	{
		// Nothing found
		*saveptr = buf;
		return buf;
	}
	else
	{
		*d = '\0';
		*saveptr = d + 1;
		return buf;
	}
}

void StarCatalog::loadCatalogs()
{
	// Load the constellation catalog
	FILE *fp = fopen(CATALOG_CONSTELLATION, "r");
	char buf[256];
	assert(fp != NULL);

	// Get the first line, and ignore it
	fgets(buf, sizeof(buf), fp);
	size_catalog_constellation = 0;

	while (true)
	{
		if (fgets(buf, sizeof(buf), fp) == NULL)
		{
			break;
		}
		if (*buf == '\0' || *buf == '\n' || *buf == '\r')
		{
			break;
		}

		char *saveptr;
		const char delim = ',';

		StarItem star;

		// Extract ID
		star.id = strtol(strtok_ch(buf, delim, &saveptr), NULL, 10);

		// Skip 4 fields
		for (int i = 0; i < 4; i++)
		{
			strtok_ch(NULL, delim, &saveptr);
		}

		// Extract BayerFlamsteed name
		strcpy(star.BFname, strtok_ch(NULL, delim, &saveptr));

		// Extract Proper name
		char *s = strtok_ch(NULL, delim, &saveptr);
		// Remove spaces
		while (*s && !isalnum(*s))
			s++;
		strcpy(star.name, s);

		// Extract RA, convert to degrees relative to 0h0m0s
		star.RA = remainder(strtod(strtok_ch(NULL, delim, &saveptr), NULL) * 15,
				360.0);

		// Extract DEC
		star.DEC = strtod(strtok_ch(NULL, delim, &saveptr), NULL);

		// Extract Distance
		star.distance = strtof(strtok_ch(NULL, delim, &saveptr), NULL);

		// Extract magnitude
		star.magnitude = strtof(strtok_ch(NULL, delim, &saveptr), NULL);

		// Extract absolute magnitude
		star.absmagnitude = strtof(strtok_ch(NULL, delim, &saveptr), NULL);

		catalog_constellation[size_catalog_constellation] = star;

		if (*star.name != '\0')
		{
			qt_common.insert(
					&catalog_constellation[size_catalog_constellation]);
		}

		qt_constellation.insert(
				&catalog_constellation[size_catalog_constellation]);

		size_catalog_constellation++;
	}
	fclose(fp);
}
//#include "mbed.h"
//extern UARTSerial serial;
//extern void stprintf(FileHandle &f, const char *, ...);
//
//static void callback(QTNode *n)
//{
//	if (n->star)
//	{
////		int i = 100000;
////		while (i--)
////			;
//		stprintf(serial,
//				"Star #%d, BF=%s, name=%s, RA=%f, DEC=%f, mag=%f, absmag=%f, depth=%d, 0x%08x\r\n",
//				n->star->id, n->star->BFname, n->star->name, n->star->RA,
//				n->star->DEC, n->star->magnitude, n->star->absmagnitude,
//				n->depth, (unsigned int) n);
//	}
//}

void StarCatalog::query_common(void (*cb)(StarItem *, void *), double ramin,
		double ramax, double decmin, double decmax, void *arg)
{
	qt_common.query(cb, NULL, ramin, ramax, decmin, decmax, arg);
}

void StarCatalog::query_allconstellations(void (*cb)(StarItem *, void *),
		double ramin, double ramax, double decmin, double decmax, void *arg)
{
	qt_constellation.query(cb, NULL, ramin, ramax, decmin, decmax, arg);
	printf("Max depth: %d\r\n", qt_constellation.maxdepth);
}
