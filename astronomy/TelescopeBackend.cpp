/*
 * TelescopeBackend.cpp
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#include <TelescopeBackend.h>

ConfigItem TelescopeBackend::configlist[] =
{
	{	.name = "Gear Ratio",
		.help = "Gearbox reduction ratio. \n1.0 means no reduction.\n2.0 means x2 reduction.",
		.type = DATATYPE_DOUBLE,
		.value = { .ddata = 1 },
		.min = { .ddata = 0 },
		.max = { .ddata = 10000 }},

	{	.name = "Worm Teeth",
		.help = "Number of teeth on the ring gear.",
		.type = DATATYPE_INT,
		.value = { .idata = 180 },
		.min = { .idata = 1 },
		.max = { .idata = 10000 }},

	{	.name = "Steps per Revolution",
		.help = "Motor steps/revolution.\nCommonly 200 or 400 for steppers.\nIf you hard-programed the microstepping,\nhere should include microstep resolution.",
		.type = DATATYPE_INT,
		.value = { .idata = 400 },
		.min = { .idata = 1 },
		.max = { .idata = 1000000 }},

{ "" } };

TelescopeBackend::TelescopeBackend()
{
	// TODO Auto-generated constructor stub

}

