#pragma once
#include "Evaluator.h"
#include "../types.h"
#include "../Board.h"
#include "../hashing/EvaluationEntry.h"
#include "../hashing/HashTable.h"

class DefaultEvaluator : public Evaluator
{
public:
	static const int PIECE_WORTH[];

	DefaultEvaluator(Board* board, u64 tableSize = 10000001);
	int evaluate();
private:
	Board* board;
	HashTable<EvaluationEntry> evalTable;
};

