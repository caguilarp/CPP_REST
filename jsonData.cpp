#include "jsonData.h"
#include "literals.h"
#include "utils.h"

jsonData::jsonData(utility::string_t ConnectionString) : Connection(ConnectionString)
{
	Connection.Connect();
}

jsonData::~jsonData()
{
	Methods.clear();
}

void jsonData::setConnectionString(utility::string_t ConnectionString)
{
	Connection.Disconnect();
	Connection.setConnectionString(ConnectionString);
	Connection.Connect();
}


web::json::value jsonData::execute(utility::string_t methodName, web::json::value Parameters)
{
	
	web::json::value answer;

	try {

		if (Methods.find(methodName) == Methods.end())
		{
			std::wcout << "Method " << methodName.data() << L" is not available" << std::endl;
			answer[LERROR] = web::json::value::string(U("Method ") + methodName + U(" is not available"));
		}
		else if (!checkParameters(methodName, Parameters) || Methods[methodName]->size() != Parameters.size())
		{
			answer[LERROR] = web::json::value::string(U("Wrong method definition"));
			answer[LDEFINITION] = defineJasonMethod(methodName);
		}
		else {
			std::wcout << L"Invoquing method " << methodName << std::endl;
			answer = executeOnDB(methodName, Parameters);
		}
	}
	catch (std::exception ex)
	{
		answer[LERROR] = web::json::value::string(U("Wrong method definition"));
	}

	return answer;
}

void jsonData::CreateMethod(int ParamCount, const wchar_t * Params, ...)
{

	va_list listParams;
	utility::string_t MethodName = utility::string_t(Params);
	va_start(listParams, Params);
	std::set<utility::string_t> *sParams = new std::set<utility::string_t>();

	for (int i = 0; i < ParamCount; i++)
	{
		Params = va_arg(listParams, const wchar_t *);
		sParams->insert(utility::string_t(Params));
	}

	va_end(listParams);
	
	Methods.insert(std::pair<utility::string_t, std::set<utility::string_t> *>(MethodName, sParams));

}

web::json::value jsonData::getMethods()
{
	web::json::value answer;
	int p = 0;

	for (auto & iMethods : Methods)
		answer[p++] = defineJasonMethod(iMethods.first);

	return answer;
}

utility::string_t jsonData::makeQueryString(utility::string_t methodName, web::json::value parameters)
{
	utility::string_t query = U("DECLARE	@return_value int; EXEC @return_value = [dbo].[sp") + methodName + U("]");
	utility::string_t paramstring = U("");

	for (auto &iter : parameters.as_object())
	{
		if (paramstring.size() > 0)
			paramstring = paramstring + U(", ");

		paramstring = paramstring + U(" @") + iter.first + U(" = ?");
	}

	query = query + paramstring + U("; select ? = @return_value");

	return query;
}

database::helperParameters * jsonData::makeParemeters(web::json::value parameters, int &paramCount, int &result, SQLLEN &result_len)
{
	
	database::helperParameters *Params = new database::helperParameters[parameters.size() + 1];

	paramCount = 0;
	
	for (auto & iter : parameters.as_object())
	{	

		const wchar_t *Data = (new std::wstring(iter.second.as_string()))->c_str();
		int Len = sizeof(Data);

		Params[paramCount].fCType = SQL_C_WCHAR;
		Params[paramCount].fSqlType = SQL_WVARCHAR;
		Params[paramCount].cbColDef = (Len == 0) ? 2 : Len;
		Params[paramCount].ibScale = 0;
		Params[paramCount].fParamType = SQL_PARAM_INPUT;
		Params[paramCount].rgbValue = (SQLPOINTER)Data;
		Params[paramCount].cbValueMax = 0;
		Params[paramCount].pcbValue = nullptr;

		paramCount++;
	}

	result_len = sizeof(result);

	Params[paramCount].fCType = SQL_C_LONG;
	Params[paramCount].fSqlType = SQL_INTEGER;
	Params[paramCount].cbColDef = 0;
	Params[paramCount].ibScale = 0;
	Params[paramCount].fParamType = SQL_PARAM_OUTPUT;
	Params[paramCount].rgbValue = reinterpret_cast<SQLPOINTER>(&result);
	Params[paramCount].cbValueMax = 0;
	Params[paramCount].pcbValue = &result_len;

	paramCount++;

	return Params;

}

web::json::value jsonData::defineJasonMethod(utility::string_t methodName)
{
	web::json::value definition;
	definition[LMETHOD] = web::json::value::string(methodName);
	web::json::value jParams;

	for (auto & iParams : *Methods[methodName])
		jParams[iParams] = web::json::value::string(U("<param_value>"));

	definition[LPARAMETERS] = jParams;

	return definition;
}

bool jsonData::checkParameters(utility::string_t methodName, web::json::value parameters)
{
	try
	{
		for (auto & iter : parameters.as_object())
			if (Methods[methodName]->find(iter.first) == Methods[methodName]->end())
				return false;
		return true;
	}
	catch (std::exception ex)
	{
		return false;
	}

}

web::json::value jsonData::executeOnDB(utility::string_t methodName, web::json::value Parameters)
{
	web::json::value result;
	web::json::value resultData;

	int ParamCount = 0;
	int Result;
	SQLLEN Result_Len;

	utility::string_t Query = makeQueryString(methodName, Parameters);
	database::helperParameters *hpParams = makeParemeters(Parameters, ParamCount, Result, Result_Len);

	if (Connection.isConnected())
	{
		if (Connection.executeQuery(Query.c_str(), hpParams, ParamCount, result))
			result[LRESULTS] = web::json::value(Result);
		else
			result[LERROR] = web::json::value(L"Can't invoke method");
	}
	else
		result[LERROR] = web::json::value(L"Can't connect to the database");

	delete[] hpParams;

	return result;
}