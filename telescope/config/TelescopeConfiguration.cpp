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
		error("ERROR: Configuration %s not defined.\r\n", configName);
		return NULL;
	}
}

PinName TelescopeConfiguration::getPin(const char* configName) {
	if (strcmp(configName, "MotorSPIMOSI") == 0){
		return D11;
	}
	else if(strcmp(configName, "MotorSPIMISO") == 0){
		return D12;
	}
	else if(strcmp(configName, "MotorSPISCLK") == 0){
		return D13;
	}
	else if(strcmp(configName, "RAMotorSPICS") == 0){
		return D10;
	}
	else if(strcmp(configName, "RAMotorStep") == 0){
		return D9;
	}
	else if(strcmp(configName, "RAMotorDir") == 0){
		return D8;
	}
	else{
		error("ERROR: Configuration %s not defined.\r\n", configName);
		return NC;
	}
}
