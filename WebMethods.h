#include <cpprest/http_listener.h>
#include <cpprest/json.h>

#ifndef WEBMETHODS_H
#define WEBMETHODS_H

void initJsonData(std::wstring ConnectionString);
void _GET(web::http::http_request request);
void _POST(web::http::http_request request);
void initMethods();
void DestroyJsonData();

#endif