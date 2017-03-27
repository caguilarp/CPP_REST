#pragma once
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <Windows.h>
#include <iostream>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <vector>
#include <stdarg.h>
#include "helperParameters.h"
#include "literals.h"

#ifndef DBHELPER_H
#define DBHELPER_H

namespace database
{

	//Class used to manage and executes queries on the database
	class dbHelper
	{
	public:
		
		//Create a new connection on the database
		dbHelper(utility::string_t ConnectionString);

		~dbHelper();

		//Connection status
		bool isConnected(); 

		void Disconnect();

		void Connect();

		void setConnectionString(utility::string_t ConnectionString);

		//Execute a standar query and return the result on a json structure
		bool executeQuery(const SQLWCHAR *QueryString, helperParameters Parameters[], int parameterCount, web::json::value &Result);

	private:
	
		SQLHENV hEnv;
		SQLHDBC hDbc;
		SQLWCHAR * pwszConnStr;
		bool _connected;
	};
}
#endif