#pragma once

#include <vector>
#include <string>
#include "Engine.h"

class UCIProtocolHandler
{
public:
	UCIProtocolHandler(Engine* engine);
	void run();
	void send(std::string s);
private:
	Engine* engine;
};

