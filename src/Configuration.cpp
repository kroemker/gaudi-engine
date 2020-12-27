#include "Configuration.h"
#include "Helpers.h"

#include <vector>
#include <iostream>
#include <fstream>

Configuration::Configuration() {
	std::ifstream configFile;

reroute:
	std::string filename = path + "gaudi.cfg";

	if (configFile.is_open()) {
		configFile.close();
	}
	configFile.open(filename);
	
	if (!configFile.is_open()) {
		std::cerr << "Configuration file \'" << filename << "\' could not be opened!" << std::endl;
		return;
	}

	std::string line;

	while (std::getline(configFile, line)) {
		std::vector<std::string> keyValue = Helpers::splitString(line, '=');
		if (keyValue.size() != 2) {
			continue;
		}

		if (keyValue[0] == "reroute") {
			path = keyValue[1];
			std::cerr << "Rerouting configuration to \'" << path + "gaudi.cfg" << "\'..." << std::endl;
			goto reroute;
		}
		else if (keyValue[0] == "engine-name") {
			engineName = keyValue[1];
		}
		else if (keyValue[0] == "eval-table-size") {
			evaluationTableSize = std::stoi(keyValue[1]);
		}
		else if (keyValue[0] == "transposition-table-size") {
			transpositionTableSize = std::stoi(keyValue[1]);
		}
		else if (keyValue[0] == "lua-eval-file") {
			luaFilename = keyValue[1];
		}
	}

	configFile.close();
}