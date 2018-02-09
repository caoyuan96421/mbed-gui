/*
 * TelescopeConfiguration.h
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPE_CONFIG_TELESCOPECONFIGURATION_H_
#define TELESCOPE_CONFIG_TELESCOPECONFIGURATION_H_

#include "mbed.h"

class TelescopeConfiguration {
public:
	TelescopeConfiguration() {
	}

	void *getConfiguration(const char *);
	PinName getPin(const char *);
};

#endif /* TELESCOPE_CONFIG_TELESCOPECONFIGURATION_H_ */
