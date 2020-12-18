#pragma once
#include "types.h"
#include <string>
#include <sstream>

class TableEntry
{
public:
	TableEntry() = default;
	virtual ~TableEntry() = default;

	virtual void dumpToStream(std::ostream& stream) {
		stream << " { hash: " << hash << " } " << std::endl;
	}

	u64 hash;
};