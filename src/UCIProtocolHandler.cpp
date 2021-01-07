#include "UCIProtocolHandler.h"
#include "Helpers.h"

#include <iostream>
#include <algorithm>

UCIProtocolHandler::UCIProtocolHandler(Engine* engine) {
	this->engine = engine;
}

void UCIProtocolHandler::run() {
	bool quit = false;
	bool think = false;
	std::string line;
	while (!quit) {
		if (think) {
			send("bestmove " + engine->move().toString());
			think = false;
		}

		std::getline(std::cin, line);
		engine->getLog()->writeMessage("Server: " + line);

		std::vector<std::string> parts = Helpers::splitString(line);

		if (parts[0] == "uci") {
			send("id name " + engine->engineName);
			send("id author kroemker");
			send("uciok");
		}
		else if (parts[0] == "ucinewgame") {
			engine->startNewGame();
		}
		else if (parts[0] == "isready") {
			send("readyok");
		}
		else if (parts[0] == "position") {
			int index = 2;
			std::string fenOrStartpos = parts[1];
			if (fenOrStartpos == "fen") {
				std::string pos = parts[2], color = parts[3], cr = parts[4], enp = parts[5], ply = parts[6], nmov = parts[7];
				engine->setBoard(pos + " " + color + " " + cr + " " + enp + " " + ply + " " + nmov);
				index = 8;
			}
			else if (fenOrStartpos == "startpos") {
				engine->setBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			}

			if (index < parts.size() && parts[index] == "moves") {
				for (index = index + 1; index < parts.size(); index++) {
					engine->doMove(parts[index]);
				}
			}
		}
		else if (parts[0] == "go") {
			think = true;
			for (int i = 0; i < parts.size(); i++) {
				if (parts[i] == "depth") {
					engine->setSearchDepth(std::stoi(parts[++i]));
				}
				else if (parts[i] == "wtime") {
					engine->setClockTime(Color::WHITE, std::stoi(parts[++i]));
				}
				else if (parts[i] == "btime") {
					engine->setClockTime(Color::BLACK, std::stoi(parts[++i]));
				}
				else if (parts[i] == "winc") {
					engine->setClockIncrement(Color::WHITE, std::stoi(parts[++i]));
				}
				else if (parts[i] == "binc") {
					engine->setClockIncrement(Color::BLACK, std::stoi(parts[++i]));
				}
				else if (parts[i] == "movetime") {
					engine->setMoveTime(std::stoi(parts[++i]));
				}
			}
		}
		else if (parts[0] == "stop") {
			think = false;
			send("bestmove " + engine->move().toString());
		}
		else if (parts[0] == "quit") {
			quit = true;
		}
		// custom debug commands
		else if (parts[0] == "debug_board") { 
			engine->showBoardDebug();
		}
	}
}

void UCIProtocolHandler::send(std::string s) {
	std::cout << s << std::endl;
	engine->getLog()->writeMessage("Engine: " + s);
}