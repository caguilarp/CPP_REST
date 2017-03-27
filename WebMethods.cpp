#include "WebMethods.h"
#include "jsonData.h"
#include "literals.h"

jsonData *MethodExecution;

void initJsonData(std::wstring ConnectionString)
{
	MethodExecution = new jsonData(ConnectionString);
}

//Used to send all methods definition
void _GET(web::http::http_request request)
{
	request.reply(web::http::status_codes::OK, MethodExecution->getMethods());
}

//Used to invoke all the methods and return the data to the client
void _POST(web::http::http_request request)
{

	web::json::value answer;

	request
		.extract_json()
		.then([&answer](pplx::task<web::json::value> task) {
		try
		{
			web::json::value jvalue = task.get();

			if (!jvalue.has_field(LMETHOD))
			{
				std::cout << "Field 'method' is not present on the string" << std::endl;
				answer[LERROR] = web::json::value::string(U("Field 'method' is not present on the string"));
				return;
			}
			else if (!jvalue.has_field(LPARAMETERS))
			{
				std::cout << "Field 'parameters' is not present on the string" << std::endl;
				answer[LERROR] = web::json::value::string(U("Field 'parameters' is not present on the string"));
				return;
			}

			auto _method = jvalue[LMETHOD];
			auto _parameters = jvalue[LPARAMETERS];

			answer = MethodExecution->execute(_method.as_string(), _parameters);

		}
		catch (web::http::http_exception const & e)
		{
			answer[LERROR] = web::json::value::string(U("Server Fault"));
			std::wcout << e.what() << std::endl;
		}
	})
		.wait();

	request.reply(web::http::status_codes::OK, answer);
}

//Initialization of the all definitions of the methods provides by the app
void initMethods()
{
	//Use this instruction to support more methods 
	MethodExecution->CreateMethod(5, METHOD_SEARCH, PARAM_TEXT, PARAM_REGISTRATION, PARAM_MAKE, PARAM_MODEL, PARAM_OWNER);

	//To support more services add here more methods - store procedures
	//MethodExecution->CreateMethod(<Parameter count>, <Method name>, <Parameter name1>, <Parameter name2>, ..., <Parameter namen>);
}

void DestroyJsonData()
{
	delete MethodExecution;
}