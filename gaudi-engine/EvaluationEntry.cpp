#include "EvaluationEntry.h"

EvaluationEntry::EvaluationEntry(u64 hash, int score) {
	this->hash = hash;
	this->score = score;
}

void EvaluationEntry::dumpToStream(std::ostream& stream) {
	stream << " { score: " << score << " } " << std::endl;
}