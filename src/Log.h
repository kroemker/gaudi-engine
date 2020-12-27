#pragma once

#include <fstream>
#include "Board.h"
#include "hashing/HashTable.h"
#include "hashing/TranspositionEntry.h"

class Log
{
public:
	Log(Board* board, HashTable<TranspositionEntry>* transTable, std::string path = "./");
	~Log();
	void writeDelimiter();
	void writePV();
	void writeBoard();
	void writeMessage(std::string str);
	std::ostream & getStream();
private:
	void writePVInternal();

	std::ofstream logFile;
	Board* board;
	HashTable<TranspositionEntry>* transTable;
};

