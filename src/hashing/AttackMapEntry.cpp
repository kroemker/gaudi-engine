#include "AttackMapEntry.h"

AttackMapEntry::AttackMapEntry(u64 hash, AttackMap attackMap) {
	this->hash = hash;
	this->attackMap = attackMap;
}

void AttackMapEntry::dumpToStream(std::ostream& stream) {
	stream << " { white: " << attackMap.map[0] << ", black: " << attackMap.map[0] << " } " << std::endl;
}