/*
 * GrayAbsEncoder.c
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#include <telescope/GrayAbsEncoder.h>

GrayAbsEncoder::GrayAbsEncoder(uint8_t count) :
		GenericAbsEncoder(1 << count), bitCount(count) {
}

GrayAbsEncoder::~GrayAbsEncoder() {
}

uint32_t GrayAbsEncoder::readPos() {
	/*Convert Gray code to Binary code*/
	uint32_t grayPos = readPosGray();
	uint32_t binPos = grayPos;
	for (grayPos >>= 1; grayPos; grayPos >>= 1)
		binPos ^= grayPos;
	return binPos;
}
