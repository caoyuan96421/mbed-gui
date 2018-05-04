/*
 * TelescopeBackend.h
 *
 *  Created on: 2018Äê4ÔÂ28ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TELESCOPEBACKEND_H_
#define TELESCOPEBACKEND_H_

typedef enum
{
	DATATYPE_INT, DATATYPE_DOUBLE, DATATYPE_STRING, DATATYPE_BOOL
} DataType;

typedef union
{
	int idata;
	double ddata;
	bool bdata;
	char strdata[32];
} DataValue;

struct ConfigItem
{
	const char *name;
	const char *help;
	DataType type;
	DataValue value;
	DataValue min;
	DataValue max;
};

class TelescopeBackend
{
public:

	static ConfigItem configlist[];

	TelescopeBackend();
	virtual ~TelescopeBackend()
	{
		// TODO Auto-generated destructor stub
	}
};

#endif /* TELESCOPEBACKEND_H_ */
