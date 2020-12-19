#pragma once

#include "Move.h"
#include "Board.h"
#include "hashing/HashTable.h"
#include "hashing/TranspositionEntry.h"

class MoveComparator
{
public:
	MoveComparator(Board* board, HashTable<TranspositionEntry>* transTable);
	bool operator()(Move& m1, Move& m2);
private:
	Board* board;
	HashTable<TranspositionEntry>* transTable;
};

