/*
 * OmronE6CPDriver.h
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_DRIVER_SERIALGRAYABSENCODER_H_
#define TELESCOPE_DRIVER_SERIALGRAYABSENCODER_H_

#include <GrayAbsEncoder.h>
#include <SerialDigitalInput.h>
#include "mbed.h"

template<uint8_t N>
class SerialGrayAbsEncoder: public GrayAbsEncoder<N>, SerialDigitalInput<N>
{
public:
	SerialGrayAbsEncoder(PinName clk, PinName load, PinName data,
	bool polarity = false) :
			GrayAbsEncoder<N>(), SerialDigitalInput<N>(clk, load, data), polarity(
					polarity)
	{
	}
	virtual ~SerialGrayAbsEncoder()
	{
	}

	bool polarity; /*If polarity=false, the input will be bit-negated before converted to raw Binary*/
	uint32_t readPosGray()
	{
		uint32_t data = SerialDigitalInput<N>::read();
		return polarity ? (((1 << N) - 1) ^ data) : data;
	}
};

#endif /* TELESCOPE_DRIVER_SERIALGRAYABSENCODER_H_ */
