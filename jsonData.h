#include <cpprest/json.h>
#include <map>
#include <set>
#include <iostream>
#include "dbHelper.h"

#ifndef JSONDATA_H
#define JSONDATA_H

//Class used to manage the relation between the restful webservices and stored on the database
class jsonData
{
public:

	//Stores the connection string to the database
	jsonData(utility::string_t ConnectionString);

	~jsonData();

	//Stores the connection string to the database
	void setConnectionString(utility::string_t ConnectionString);

	//Executes a method stored on the database
	web::json::value execute(utility::string_t methodName, web::json::value Parameters);

	//Used to create a specific method definition
	void CreateMethod(int ParamCount, const wchar_t * Params, ...);

	//Get all methods definitions
	web::json::value getMethods();

private:

	//Database connection control
	database::dbHelper Connection;

	//Method stored in the database
	std::map<utility::string_t, std::set<utility::string_t> *> Methods;

	//Used to send a pretty definition of all the method stored
	web::json::value defineJasonMethod(utility::string_t methodName);

	//Check if an executed method match with the definition (parameters)
	bool checkParameters(utility::string_t methodName, web::json::value parameters);

	//Executes a method stored on the database
	web::json::value executeOnDB(utility::string_t methodName, web::json::value Parameters);

	//Create a standard query based on the method stored on the database
	utility::string_t makeQueryString(utility::string_t methodName, web::json::value parameters);

	//Create a structure used to pass all the parameters to the database
	database::helperParameters * makeParemeters(web::json::value parameters, int &paramCount, int &result, SQLLEN &result_len);

};

#endif 