#include <iostream>
#include <vector>

#include "Engine.h"
#include "UCIProtocolHandler.h"
#include "ClockHandler.h"

int main(int argc, char* argv[])
{
	Engine engine;
	UCIProtocolHandler uci(&engine);
	bool runTests = false;
	bool selfPlay = false;
	bool runUCI = true;

	int time = 1000;
	int increment = -1;

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-t") == 0) {
			runTests = true;
			runUCI = false;
		}
		if (strcmp(argv[i], "-uci") == 0) {
			runUCI = true;
		}
		if (strcmp(argv[i], "-s") == 0) {
			selfPlay = true;
			runUCI = false;
			i++;
			if (i < argc) {
				time = atoi(argv[i]) * 1000;
			}
			i++;
			if (i < argc) {
				increment = atoi(argv[i]) * 1000;
			}
		}
	}
	if (runTests) {
		engine.runTests();
	}
	if (selfPlay) {
		engine.playSelf(increment == -1 ? ClockHandler::MOVETIME : ClockHandler::NORMAL, increment == -1 ? time : 60 * time, increment < 0 ? 0 : increment);
	}
	if (runUCI) {
		uci.run();
	}
}