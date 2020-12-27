#pragma once

#include <string>

#include <lua.hpp>

#include "Configuration.h"
#include "Board.h"
#include "Searcher.h"
#include "Move.h"
#include "Log.h"
#include "PGN.h"
#include "ClockHandler.h"
#include "hashing/HashTable.h"
#include "hashing/TranspositionEntry.h"
#include "evaluation/Evaluator.h"

class Engine
{
public:
	std::string engineName;

	Engine(Configuration* configuration);
	~Engine();
	Move move();
	void startNewGame();
	void setBoard(std::string fen);
	void doMove(std::string move);
	void playSelf(int clockMode = ClockHandler::MOVETIME, int time = 1000, int increment = 0);
	Log* getLog();
	void setClockTime(int color, int timeMs);
	void setClockIncrement(int color, int timeMs);
	void setMoveTime(int timeMs);
	void setSearchDepth(int depth);
	void runTests();
	void evaluatePosition(std::string fen);
private:
	Board board;
	Searcher searcher;
	HashTable<TranspositionEntry> transTable;
	ClockHandler clockHandler;
	Log log;
	PGN pgn;
	Evaluator* evaluator;
	lua_State* luaState;
	int searchDepth;
};

