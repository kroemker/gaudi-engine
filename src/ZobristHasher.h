#pragma once

#include "types.h"
#include "Move.h"

class Board;

class ZobristHasher
{	
public:
	//2 players * 64 squares * 6 different pieces + 16 castling right permutations + 8 files of the enpassant square + 1 black to move
	static const int tableSize = 2 * 64 * 6 + 16 + 8 + 1;

	ZobristHasher(Board* board);
	void hashNew();
	void updateHash(Move& m);
	u64 getHash();
private:
	Board* board; 
	u64 hash;
	u64 table[tableSize];
	int castlingRightIndex;
};

