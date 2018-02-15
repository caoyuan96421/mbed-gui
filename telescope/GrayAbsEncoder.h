/*
 * GrayAbsEncoder.h
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_GRAYABSENCODER_H_

#define TELESCOPE_GRAYABSENCODER_H_

#include <AbsEncoder.h>

template<uint8_t N>
class GrayAbsEncoder: public AbsEncoder<(1 << N)>
{
public:
	GrayAbsEncoder()
	{
	}
	virtual ~GrayAbsEncoder()
	{
	}

	virtual uint32_t readPosGray() = 0;
	virtual uint32_t readPos()
	{
		/*Convert Gray code to Binary code*/
		uint32_t grayPos = readPosGray();
		uint32_t binPos = grayPos;
		for (grayPos >>= 1; grayPos; grayPos >>= 1)
			binPos ^= grayPos;
		return binPos;
	}

	uint8_t getBits() const
	{
		return N;
	}
};

#endif /* TELESCOPE_GRAYABSENCODER_H_ */
