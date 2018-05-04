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
	double RA;
	double DEC;
	char name[20];
	float distance;
	float magnitude;
	float absmagnitude;
};

inline double min(double x, double y)
{
	return (x < y) ? x : y;
}

inline double max(double x, double y)
{
	return (x > y) ? x : y;
}

struct QTNode
{
	StarItem *star;
	QTNode *daughters[4];
	double ral, rar;
	double decl, decr;
	bool isleaf;
	int depth;

	QTNode(double xl = 0, double xr = 0, double yl = 0, double yr = 0) :
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

	QTNode *quadrant(double ra, double dec)
	{
		if (ra < ral || ra > rar || dec < decl || dec > decr)
		{
			// Not found
			return NULL;
		}
		double ram = (ral + rar) / 2;
		double decm = (decl + decr) / 2;

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
			double rl = (q & 1) ? ral : ram;
			double rr = (q & 1) ? ram : rar;
			double dl = (q & 2) ? decl : decm;
			double dr = (q & 2) ? decm : decr;
			daughters[q] = this->malloc();
			*daughters[q] = QTNode(rl, rr, dl, dr);
			daughters[q]->depth = depth + 1;
			return daughters[q];
		}
	}

	bool intersects(double rl, double rr, double dl, double dr)
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

	bool insert(StarItem *newstar)
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

	void query(void (*cb)(StarItem *, void *), QTNode *p, double ral,
			double rar, double decl, double decr, void *arg)
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
				query(cb, p, rar, 180.0, decl, decr, arg);
				query(cb, p, -180.0, ral, decl, decr, arg);
				return;
			}
		}

		// Check star at current node
		if (p->star)
		{
			if (p->star->RA <= rar && p->star->RA >= ral && p->star->DEC <= decr
					&& p->star->DEC >= decl)
			{
				cb(p->star, arg);
			}
		}

		// Now check daughters
		if (!p->isleaf)
		{
			for (int i = 0; i < 4; i++)
			{
				if (p->daughters[i]
						&& p->daughters[i]->intersects(ral, rar, decl, decr))
				{
					query(cb, p->daughters[i], ral, rar, decl, decr, arg);
				}
			}
		}

	}
};

class StarCatalog
{
public:
	static StarCatalog &getInstance()
	{
		static StarCatalog instance;
		return instance;
	}

	void query_common(void (*cb)(StarItem *, void *), double ramin,
			double ramax, double decmin, double decmax, void *arg);
	void query_allconstellations(void (*cb)(StarItem *, void *), double ramin,
			double ramax, double decmin, double decmax, void *arg);

private:
	StarCatalog();
	virtual ~StarCatalog()
	{
	}

	static void loadCatalogs();
};

#endif /* ASTRONOMY_STARCATALOG_H_ */
