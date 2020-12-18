#pragma once
#include "Board.h"
#include "Evaluator.h"
#include "MoveComparator.h"
#include "HashTable.h"
#include "ZobristHasher.h"
#include "Log.h"
#include <chrono>

class Searcher
{
public:
	static const int MAX_SCORE = 1000000000;
	static const int MATE_SCORE = 1000000;

	Searcher(Board* board, Evaluator* evaluator, HashTable<TranspositionEntry>* transTable, Log* log);
	void search(int depth, int timeLimitMs);
	int pvSearchRoot(int depth);
	Move getBestMove();
	int pvSearch(int alpha, int beta, int depth, bool pvNode); 
	int quiesce(int alpha, int beta);

	void checkTimeUp();

	void test(std::string fen, int depth);
	void assertBoardHash(u64 should);
private:
	Board* board;
	Evaluator* evaluator;
	HashTable<TranspositionEntry>* transTable;
	Log* log;
	MoveComparator moveComparator;
	Move bestMove;

	int nodes;
	int quiesceNodes;
	int tableHits;
	int quiesceTableHits;
	int evaluations;

	std::chrono::steady_clock::time_point beginSearch;
	int timeLimit;
	bool timeUp;
};

