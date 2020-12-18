#pragma once
#include "types.h"
#include "TableEntry.h"

class EvaluationEntry : public TableEntry
{
public:
	EvaluationEntry() = default;
	EvaluationEntry(u64 hash, int score);
	~EvaluationEntry() = default;

	void dumpToStream(std::ostream& stream);

	int score;
};

