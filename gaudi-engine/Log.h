#pragma once

#include <fstream>
#include "Board.h"
#include "HashTable.h"
#include "TranspositionEntry.h"

class Log
{
public:
	Log(Board* board, HashTable<TranspositionEntry>* transTable);
	~Log();
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
