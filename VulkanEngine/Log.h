#pragma once
#include <string>
class CLog
{
public:
	CLog();
	~CLog();

	void warning(std::string str);
	void error();

};

