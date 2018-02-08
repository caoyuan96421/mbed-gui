/*
 * TelescopeConfiguration.cpp
 *
 *  Created on: 2018Äê2ÔÂ7ÈÕ
 *      Author: caoyuan9642
 */

#include <telescope/config/TelescopeConfiguration.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mbed.h"

/**
 * Default configurations for the telescope. Can be overriden for
 *
 */
void *TelescopeConfiguration::getConfiguration(const char* configName) {
	if(strcmp(configName, "PECEncoderCLK") == 0){
		static DigitalOut peCLK = DigitalOut(D6);
		return &peCLK;
	}
	else if(strcmp(configName, "PECEncoderLD") == 0){
		static DigitalOut peLD = DigitalOut(D4);
		return &peLD;
	}
	else if(strcmp(configName, "PECEncoderDATA") == 0){
		static DigitalIn peDATA = DigitalIn(D5);
		return &peDATA;
	}
	else{
		fprintf(stderr, "ERROR: Configuration %s not defined.", configName);
		return NULL;
	}
}
