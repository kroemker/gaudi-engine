#pragma once
#include "TableEntry.h"
#include "../AttackMap.h"

class AttackMapEntry : public TableEntry
{
public:
	AttackMapEntry() = default;
	AttackMapEntry(u64 hash, AttackMap attackMap);
	~AttackMapEntry() = default;

	void dumpToStream(std::ostream& stream);

	AttackMap attackMap;
};

