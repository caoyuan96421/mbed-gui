/*
 * GenericAbsEncoder.h
 *
 *	Interface for reading position from an absolute encoder
 *
 *  Created on: 2018��2��7��
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_GENERICABSENCODER_H_
#define TELESCOPE_GENERICABSENCODER_H_

#include <stdint.h>

class GenericAbsEncoder {
protected:
	uint32_t maxCount;

public:
	GenericAbsEncoder(uint32_t mc);
	virtual ~GenericAbsEncoder();

	virtual uint32_t readPos() = 0;
	virtual void zero() {
	}

	uint32_t getMaxCount() const {
		return maxCount;
	}
};

#endif /* TELESCOPE_GENERICABSENCODER_H_ */
