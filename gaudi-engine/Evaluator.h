#pragma once

#include "types.h"
#include "Board.h"
#include "EvaluationEntry.h"
#include "HashTable.h"

class Evaluator
{
public:
	static const int PIECE_WORTH[];

	Evaluator(Board* board, u64 tableSize = 1006003);
	virtual int evaluate();
private:
	Board* board;
	HashTable<EvaluationEntry> evalTable;
};

