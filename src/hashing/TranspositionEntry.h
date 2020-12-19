#pragma once
#include "TableEntry.h"
#include "../Move.h"

class TranspositionEntry : public TableEntry
{
public:
	static const u8 HASH_EXACT = 1;
	static const u8 HASH_BETA = 2;
	static const u8 HASH_ALPHA = 3;
	static const u8 HASH_QUIESCE = 4;

	TranspositionEntry() = default;
	TranspositionEntry(u64 hash, u8 depth, int score, u8 flag, Move bestMove);
	~TranspositionEntry() = default;

	void dumpToStream(std::ostream& stream);

	u8 depth;
	int score;
	u8 flag;
	Move bestMove;
};

