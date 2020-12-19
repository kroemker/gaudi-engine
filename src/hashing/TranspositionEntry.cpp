#include "TranspositionEntry.h"


TranspositionEntry::TranspositionEntry(u64 hash, u8 depth, int score, u8 flag, Move bestMove) {
	this->hash = hash;
	this->depth = depth;
	this->score = score;
	this->flag = flag;
	this->bestMove = bestMove;
}

void TranspositionEntry::dumpToStream(std::ostream& stream) {
	stream << " { score: " << score <<	", move: " << bestMove.toString() << ", depth: " << (int)depth << ", flag: " << (int)flag << " } " << std::endl;
}