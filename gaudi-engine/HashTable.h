#pragma once

#include <string>
#include <fstream>
#include "types.h"
#include "Move.h"
#include "TableEntry.h"
#include "TranspositionEntry.h"
#include "EvaluationEntry.h"

template <class T>
class HashTable
{
public:
	HashTable(u64 capacity, std::string name = "table") {
		this->capacity = capacity;
		this->name = name;
		entries = new T[capacity]();
	}

	~HashTable() {
		delete[] entries;
	}

	void store(T entry) {
		u64 index = entry.hash % capacity;
		entries[index] = entry;
	}

	T* find(u64 hash) {
		return &entries[hash % capacity];
	}

	void dumpToFile(std::string filename) {
		std::ofstream fs;
		fs.open(filename);
		dumpToStream(fs);
		fs.close();
	}

	void dumpToStream(std::ostream& stream) {
		for (int i = 0; i < capacity; i++) {
			if (entries[i].hash != 0) {
				stream << name << "[" << i << "] =";
				entries[i].dumpToStream(stream);
			}
		}
	}
private:
	u64 capacity;
	std::string name;
	T* entries;
};

