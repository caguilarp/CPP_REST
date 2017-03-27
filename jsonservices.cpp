#include <windows.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <string>
#include "jsonData.h"
#include "utils.h"
#include "literals.h"
#include "WebMethods.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;
using namespace utility;

int main(int argc, char* argv[])
{

	if (argc == PARAM_COUNT)
	{
		http_listener lisetener(chartoWString(argv[URL_PARAM]) + METHOD_PATH);
		initJsonData(chartoWString(argv[CONNECTION_PARAM]));
		initMethods();

		lisetener.support(methods::GET, _GET);
		lisetener.support(methods::POST, _POST);

		try
		{
			lisetener
				.open()
				.then([&lisetener]() { cout << "Starting Services\n"; })
				.wait();

			while (true);
		}
		catch (exception const & e)
		{
			wcout << e.what() << endl;
			DestroyJsonData();
		}
	}
	else
		cout << "Invalid Parameters" << endl;


	return 0;
}






