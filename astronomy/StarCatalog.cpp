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

#define MAX_STAR 42000

//const char CATALOG_COMMON[] = "/sdcard/hygcommon.csv";
const char CATALOG_CONSTELLATION[] = "/sdcard/hygmag8.csv";
const char CATALOG_ALL[] = "/sdcard/hygfull.csv";

// Catalog of constellation stars
__attribute__((section (".sdram")))
                        StarItem catalog_constellation[MAX_STAR];

unsigned int size_catalog_constellation;

QuadTree qt_common;
QuadTree qt_constellation;

__attribute__((section (".sdram")))
               QTNode _nodefactory[MAX_STAR];

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
		star.RA = remainderf(
				strtof(strtok_ch(NULL, delim, &saveptr), NULL) * 15.0f, 360.0f);

		// Extract DEC
		star.DEC = strtof(strtok_ch(NULL, delim, &saveptr), NULL);

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

void StarCatalog::query_common(void (*cb)(StarItem *, void *), float ramin,
		float ramax, float decmin, float decmax, void *arg, float maxmag)
{
	qt_common.query(cb, NULL, ramin, ramax, decmin, decmax, arg, maxmag);
}

void StarCatalog::query_all(void (*cb)(StarItem *, void *), float ramin,
		float ramax, float decmin, float decmax, void *arg, float maxmag)
{
	qt_constellation.query(cb, NULL, ramin, ramax, decmin, decmax, arg, maxmag);
//	printf("Max depth: %d\r\n", qt_constellation.maxdepth);
}

bool QuadTree::insert(StarItem* newstar)
{
	if (!newstar)
	{
		return false;
	}
	QTNode *p = &head;
	while (p->star)
	{
		// Find quadrant and dive in
		p = p->quadrant(newstar->RA, newstar->DEC);
		if (p == NULL)
			return false; // Failed
	}

	p->star = newstar;
	if (p->depth > maxdepth)
		maxdepth = p->depth;
	return true;
}

void QuadTree::query(void (*cb)(StarItem*, void*), QTNode* p, float ral,
		float rar, float decl, float decr, void* arg, float maxmag)
{
	if (p == NULL)
	{
		p = &head;
	}

	if (p == &head)
	{
		// Check
		ral = remainder(ral, 360.0);
		rar = remainder(rar, 360.0);
		if (decl > decr)
		{
			double temp = decr;
			decr = decl;
			decl = temp;
		}
		if (decl < -90.0)
			decl = -90.0;
		if (decr > 90.0)
			decr = 90.0;
		if (ral > rar)
		{
			// Crossing 180/-180 RA, divide into two
			query(cb, p, rar, 180.0, decl, decr, arg, maxmag);
			query(cb, p, -180.0, ral, decl, decr, arg, maxmag);
			return;
		}
	}

// Check star at current node
	if (p->star && p->star->magnitude < maxmag)
	{
		if (p->star->RA <= rar && p->star->RA >= ral && p->star->DEC <= decr
				&& p->star->DEC >= decl)
		{
			cb(p->star, arg);
		}
	}
	else{
		// If current node is below magnitude threshhold, then no more searching
		// Because its children must be even lower magnitude
		return;
	}

// Now check daughters
	if (!p->isleaf)
	{
		for (int i = 0; i < 4; i++)
		{
			if (p->daughters[i]
					&& p->daughters[i]->intersects(ral, rar, decl, decr))
			{
				query(cb, p->daughters[i], ral, rar, decl, decr, arg, maxmag);
			}
		}
	}
}
