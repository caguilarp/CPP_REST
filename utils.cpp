#include "utils.h"

std::wstring chartoWString(char *s)
{
	std::string Convert = s;
	return std::wstring(Convert.begin(), Convert.end());
}

std::string wstringToString(std::wstring s)
{
	return std::string(s.begin(), s.end());
}