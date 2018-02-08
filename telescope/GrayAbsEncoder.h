/*
 * GrayAbsEncoder.h
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_GRAYABSENCODER_H_
#define TELESCOPE_GRAYABSENCODER_H_

#include "GenericAbsEncoder.h"

class GrayAbsEncoder: public GenericAbsEncoder {
protected:
	uint8_t bitCount;
	virtual uint32_t readPosGray() = 0;

public:
	GrayAbsEncoder(uint8_t count);
	virtual ~GrayAbsEncoder();

	virtual uint32_t readPos();

	uint8_t getBitCount() const {
		return bitCount;
	}
};

#endif /* TELESCOPE_GRAYABSENCODER_H_ */
