#pragma once

#include <string>
#include "Board.h"
#include "Searcher.h"
#include "Move.h"
#include "Log.h"
#include "hashing/HashTable.h"
#include "hashing/TranspositionEntry.h"
#include "ClockHandler.h"

class Engine
{
public:
	static const std::string engineName;

	Engine();
	Move move();
	void startNewGame();
	void setBoard(std::string fen);
	void doMove(std::string move);
	Log* getLog();
	void setClockTime(int color, int timeMs);
	void setClockIncrement(int color, int timeMs);
	void setMoveTime(int timeMs);
	void setSearchDepth(int depth);
	void runTests();
private:
	Board board;
	Searcher searcher;
	Evaluator evaluator;
	HashTable<TranspositionEntry> transTable;
	ClockHandler clockHandler;
	Log log;
	int searchDepth;
};

