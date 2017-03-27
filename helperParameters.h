#include <Windows.h>
#include <iostream>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

#ifndef HELPERPARAMETERS_H
#define HELPERPARAMETERS_H

namespace database
{
	//Query Parameters Structure
	struct helperParameters
	{
		SQLSMALLINT fCType;
		SQLSMALLINT fSqlType;
		SQLUINTEGER cbColDef;
		SQLSMALLINT fParamType;
		SQLSMALLINT ibScale;
		SQLPOINTER rgbValue;
		SQLINTEGER cbValueMax;
		SQLLEN *pcbValue;
	};
}

#endif