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
#include <cfloat>
#include "HYGMag8.h"

QuadTree qt_common;
QuadTree qt_constellation;

__attribute__((section (".sdram")))
                                       QTNode _nodefactory[STAR_CATALOG_SIZE];

__attribute__((section (".sdram")))
                                       HashMap::HashMapNode _hashmapfactory[STAR_CATALOG_SIZE];

static int _nodecount = 0;
static int _hashcount = 0;

HashMap starIndex;

static inline float sqr(float x)
{
	return x * x;
}

QTNode* QTNode::malloc()
{
	if (_nodecount == STAR_CATALOG_SIZE)
		return NULL;
	return &_nodefactory[_nodecount++];
}

StarCatalog::StarCatalog()
{
	constructTree();
}

StarInfo* StarCatalog::searchByCoordinates(float ra, float dec, float maxdist)
{
	return qt_constellation.findClosest(ra, dec, maxdist);
}

StarInfo* StarCatalog::searchByID(int id)
{
	return starIndex[id];
}

void StarCatalog::constructTree()
{

	for (unsigned int i = 0; i < STAR_CATALOG_SIZE; i++)
	{
		StarInfo &star = star_catalog_hyg[i];
		if (star.id < 0)
		{
			break;
		}
		if (*star.name != '\0')
		{
			qt_common.insert(&star);
		}

		qt_constellation.insert(&star);
		starIndex[star.id] = &star;
	}
}

void StarCatalog::query_common(void (*cb)(StarInfo *, void *), float ramin, float ramax, float decmin, float decmax, void *arg, float maxmag)
{
	qt_common.query(cb, ramin, ramax, decmin, decmax, arg, maxmag);
}

void StarCatalog::query_all(void (*cb)(StarInfo *, void *), float ramin, float ramax, float decmin, float decmax, void *arg, float maxmag)
{
	qt_constellation.query(cb, ramin, ramax, decmin, decmax, arg, maxmag);
}

bool QuadTree::insert(StarInfo* newstar)
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
	return true;
}

void QuadTree::query(void (*cb)(StarInfo*, void*), float ral, float rar, float decl, float decr, void* arg, float maxmag)
{
	// Check
	ral = remainderf(ral, 360.0f);
	rar = remainderf(rar, 360.0f);
	if (decl > decr)
	{
		float temp = decr;
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
		_query(cb, &head, ral, 180.0f, decl, decr, arg, maxmag);
		_query(cb, &head, -180.0f, rar, decl, decr, arg, maxmag);
		return;
	}
	else
	{
		_query(cb, &head, ral, rar, decl, decr, arg, maxmag);
	}

}

StarInfo* QuadTree::search(float ra, float dec, float maxdist)
{
	return _search(&head, ra, dec, maxdist);
}

StarInfo* QuadTree::_search(QTNode* p, float ra, float dec, float maxdist)
{
	if (!p->star)
		return NULL;
	if (fabsf(remainderf(p->star->DEC - dec, 360)) < maxdist && fabsf(remainderf(p->star->RA - ra, 360)) < maxdist)
	{
		return p->star;
	}
	if (!p->isleaf)
	{
		// Search daughters in turn
		for (int i = 0; i < 4; i++)
		{
			if (p->daughters[i] && p->daughters[i]->intersects(ra - maxdist, ra + maxdist, dec - maxdist, dec + maxdist))
			{
				StarInfo *ret = _search(p->daughters[i], ra, dec, maxdist);
				if (ret)
					return ret;
			}
		}
	}
	return NULL; // Nothing found
}

StarInfo* QuadTree::findClosest(float ra, float dec, float maxdist)
{
	return _find(&head, ra, dec, maxdist);
}

StarInfo* QuadTree::_find(QTNode* p, float ra, float dec, float &maxdist)
{
	if (!p->star)
		return NULL;
	StarInfo *closest = NULL;
	float d2 = sqr(remainderf(p->star->DEC - dec, 360)) + sqr(remainderf(p->star->RA - ra, 360));
	float m2 = sqr(maxdist);
	if (d2 < m2)
	{
		closest = p->star;
		maxdist = sqrtf(d2);
	}

	if (!p->isleaf)
	{
		// Search daughters in turn
		for (int i = 0; i < 4; i++)
		{
			if (p->daughters[i] && p->daughters[i]->intersects(ra - maxdist, ra + maxdist, dec - maxdist, dec + maxdist))
			{
				float m=maxdist;
				StarInfo *ret = _find(p->daughters[i], ra, dec, m);
				if (ret && m<maxdist){
					maxdist = m;
					closest = ret;
				}
			}
		}
	}
	return closest;
}

void QuadTree::_query(void (*cb)(StarInfo*, void*), QTNode* p, float ral, float rar, float decl, float decr, void* arg, float maxmag)
{

// Check star at current node
	if (p->star && p->star->magnitude < maxmag)
	{
		if (p->star->RA <= rar && p->star->RA >= ral && p->star->DEC <= decr && p->star->DEC >= decl)
		{
			cb(p->star, arg);
		}
	}
	else
	{
		// If current node is below magnitude threshhold, then no more searching
		// Because its children must be even lower magnitude
		return;
	}

// Now check daughters
	if (!p->isleaf)
	{
		for (int i = 0; i < 4; i++)
		{
			if (p->daughters[i] && p->daughters[i]->intersects(ral, rar, decl, decr))
			{
				_query(cb, p->daughters[i], ral, rar, decl, decr, arg, maxmag);
			}
		}
	}
}

HashMap::HashMap()
{
	for (int i = 0; i < HASH_SIZE; i++)
	{
		list[i] = NULL;
	}
}

HashMap::~HashMap()
{
	for (int i = 0; i < HASH_SIZE; i++)
	{
		HashMapNode *p = list[i];
		while (p)
		{
			HashMapNode *q = p;
			delete p;
			p = q;
		}
	}
}

StarInfo*& HashMap::operator [](int id)
{
	int k = id % HASH_SIZE;
	HashMapNode *p = list[k], *q = NULL;
	while (p && p->id != id)
		p = (q = p)->next;
	if (!p)
	{
		p = malloc();
		p->id = id;
//		p->next = list[k];
//		list[k] = p; // Insert at the beginning of the list
		p->next = NULL;
		if (q)
			q->next = p;
		else
			list[k] = p; // Insert at the end of the list
	}
	return p->star;
}

HashMap::HashMapNode *HashMap::malloc()
{
	if (_hashcount == STAR_CATALOG_SIZE)
	{
		return NULL;
	}
	return &_hashmapfactory[_hashcount++];
}

