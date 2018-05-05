/*
 * StarCatalog.h
 *
 *  Created on: 2018Äê5ÔÂ2ÈÕ
 *      Author: caoyuan9642
 */

#ifndef ASTRONOMY_STARCATALOG_H_
#define ASTRONOMY_STARCATALOG_H_

#include <cstdlib>
#include <cmath>

struct StarItem
{
	int id;
	char BFname[12];
	float RA;
	float DEC;
	char name[20];
	float distance;
	float magnitude;
	float absmagnitude;
};

inline float min(float x, float y)
{
	return (x < y) ? x : y;
}

inline float max(float x, float y)
{
	return (x > y) ? x : y;
}

struct QTNode
{
	StarItem *star;
	QTNode *daughters[4];
	float ral, rar;
	float decl, decr;
	bool isleaf;
	int depth;

	QTNode(float xl = 0, float xr = 0, float yl = 0, float yr = 0) :
			star(NULL), ral(xl), rar(xr), decl(yl), decr(yr), isleaf(true), depth(
					0)
	{
		daughters[0] = NULL;
		daughters[1] = NULL;
		daughters[2] = NULL;
		daughters[3] = NULL;
	}

	~QTNode()
	{
		for (int i = 0; i < 4; i++)
		{
			if (daughters[i])
				daughters[i]->~QTNode();
		}
	}

	QTNode *malloc();

	QTNode *quadrant(float ra, float dec)
	{
		if (ra < ral || ra > rar || dec < decl || dec > decr)
		{
			// Not found
			return NULL;
		}
		float ram = (ral + rar) * 0.5;
		float decm = (decl + decr) * 0.5;

		int q;
		if (dec >= decm)
		{
			if (ra >= ram)
				q = 0;
			else
				q = 1;
		}
		else
		{
			if (ra >= ram)
				q = 2;
			else
				q = 3;
		}
		if (daughters[q])
		{
			// Found
			return daughters[q];
		}
		else
		{
			// Create
			isleaf = false;
			float rl = (q & 1) ? ral : ram;
			float rr = (q & 1) ? ram : rar;
			float dl = (q & 2) ? decl : decm;
			float dr = (q & 2) ? decm : decr;
			daughters[q] = this->malloc();
			*daughters[q] = QTNode(rl, rr, dl, dr);
			daughters[q]->depth = depth + 1;
			return daughters[q];
		}
	}

	bool intersects(float rl, float rr, float dl, float dr)
	{
		if (max(rl, rr) <= ral || min(rl, rr) >= rar)
			return false;
		if (max(dl, dr) <= decl || min(dl, dr) >= decr)
			return false;
		return true;
	}
};

struct QuadTree
{
	QTNode head;
	int maxdepth;

	QuadTree() :
			head(-180.0, 180.0, -90.0, 90.0), maxdepth(0)
	{
	}

	bool insert(StarItem *newstar);
	void query(void (*cb)(StarItem *, void *), QTNode *p, float ral, float rar,
			float decl, float decr, void *arg, float maxmag);
};

class StarCatalog
{
public:
	static StarCatalog &getInstance()
	{
		static StarCatalog instance;
		return instance;
	}

	void query_common(void (*cb)(StarItem *, void *), float ramin, float ramax,
			float decmin, float decmax, void *arg, float maxmag);
	void query_all(void (*cb)(StarItem *, void *), float ramin, float ramax,
			float decmin, float decmax, void *arg, float maxmag);

private:
	StarCatalog();
	virtual ~StarCatalog()
	{
	}

	static void loadCatalogs();
};

#endif /* ASTRONOMY_STARCATALOG_H_ */
