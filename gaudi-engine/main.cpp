#include <iostream>
#include <vector>

#include "Engine.h"
#include "UCIProtocolHandler.h"

int main(int argc, char* argv[])
{
	Engine engine;
	UCIProtocolHandler uci(&engine);
	bool runTests = false;
	bool runUCI = true;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-t") == 0) {
			runTests = true;
			runUCI = false;
		}
		if (strcmp(argv[i], "-uci") == 0) {
			runUCI = true;
		}
	}
	if (runTests) {
		engine.runTests();
	}
	if (runUCI) {
		uci.run();
	}
}