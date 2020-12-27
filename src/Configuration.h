#pragma once

#include <string>

class Configuration
{
public:
	Configuration();

	std::string engineName = "Flagfish";
	std::string path = "./";
	int transpositionTableSize = 1000001;
	int evaluationTableSize = 1000001;
	std::string luaFilename;
};