#include "Log.h"

#include <iostream>
#include <sstream>
#include <ctime>

Log::Log(Board* board, HashTable<TranspositionEntry>* transTable, std::string path) {
	this->board = board;
	this->transTable = transTable;
	std::time_t now = std::time(nullptr);
	tm* localTime = localtime(&now);
	std::stringstream ss;
	ss << path << "log_" << 1900 + localTime->tm_year << "-" << 1 + localTime->tm_mon << "-" 
	   << localTime->tm_mday << "_" << localTime->tm_hour << "-" 
	   << localTime->tm_min << "-" << localTime->tm_sec << ".txt";
	logFile.open(ss.str());
}

Log::~Log() {
	if (logFile.is_open()) {
		logFile.close();
	}
}

void Log::writeDelimiter() {
	logFile << "-------------------------------------------------------------" << std::endl;
}

void Log::writePV() {
	logFile << "PV: ";
	writePVInternal();
	logFile << std::endl;
}

void Log::writePVInternal() {
	TranspositionEntry* e = transTable->find(board->getHash());
	if (e->hash == board->getHash() && e->flag == TranspositionEntry::HASH_EXACT) {
		logFile << board->getMoveStringAlgebraic(e->bestMove) << " ";
		board->makeMove(e->bestMove);
		if (!board->isRepetition()) {
			writePVInternal();
		}
		board->unmakeMove(e->bestMove);
	}
}

void Log::writeBoard() {
	board->print(logFile);
}
void Log::writeMessage(std::string str) {
	logFile << str << std::endl;
}

std::ostream& Log::getStream() {
	return logFile;
}