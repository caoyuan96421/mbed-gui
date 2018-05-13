/*
 * SkyCulture.h
 *
 *  Created on: 2018Äê5ÔÂ9ÈÕ
 *      Author: caoyuan9642
 */

#ifndef ASTRONOMY_SKYCULTURE_H_
#define ASTRONOMY_SKYCULTURE_H_

#include "StarCatalog.h"

class SkyCulture
{
public:
	typedef enum
	{
		Aql = 0,
		And,
		Scl,
		Ara,
		Lib,
		Cet,
		Ari,
		Pyx,
		Boo,
		Cae,
		Cha,
		Cnc,
		Cap,
		Car,
		Cas,
		Cen,
		Cep,
		Com,
		Cvn,
		Aur,
		Col,
		Cir,
		Crt,
		CrA,
		CrB,
		Crv,
		Cru,
		Cyg,
		Del,
		Dor,
		Dra,
		Nor,
		Eri,
		Sge,
		For,
		Gem,
		Cam,
		CMa,
		UMa,
		Gru,
		Her,
		Hor,
		Hya,
		Hyi,
		Ind,
		Lac,
		Mon,
		Lep,
		Leo,
		Lup,
		Lyn,
		Lyr,
		Ant,
		Mic,
		Mus,
		Oct,
		Aps,
		Oph,
		Ori,
		Pav,
		Peg,
		Pic,
		Per,
		Equ,
		CMi,
		LMi,
		Vul,
		UMi,
		Phe,
		Psc,
		PsA,
		Vol,
		Pup,
		Ret,
		Sgr,
		Sco,
		Sct,
		Ser,
		Sex,
		Men,
		Tau,
		Tel,
		Tuc,
		Tri,
		Tra,
		Aqr,
		Vir,
		Vel,
	} Constellation;

	struct Segment
	{
		StarInfo *star1;
		StarInfo *star2;
		float x1;
		float y1;
		float z1;
		float x2;
		float y2;
		float z2;
		Segment *next;
		Segment(int i1 = 0, int i2 = 0, Segment *n = NULL) :
				star1(NULL), star2(NULL), x1(0), y1(0), z1(0), x2(0), y2(0), z2(0), next(n)
		{
		}
	};

	struct Iterator
	{
		int constell;
		Segment *segment;
	};

	static SkyCulture &getInstance()
	{
		static SkyCulture inst;
		return inst;
	}

	static void resetIterator(Iterator &it)
	{
		getInstance()._resetIterator(it);
	}

	static Segment *nextSegment(Iterator &it)
	{
		return getInstance()._nextSegment(it);
	}

private:
	SkyCulture();
	virtual ~SkyCulture();

	void _resetIterator(Iterator &it);
	Segment *_nextSegment(Iterator &it);

};

#endif /* ASTRONOMY_SKYCULTURE_H_ */
