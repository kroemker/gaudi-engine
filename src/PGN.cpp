#include "PGN.h"

#include <iostream>
#include <fstream>
#include <ctime>

PGN::PGN() {
	numMoves = 2;
}

void PGN::setDate(int year, int month, int day) {
	this->year = year;
	this->month = month;
	this->day = day;
}
void PGN::setResult(std::string result) {
	this->result = result;
	moves << result;
}
void PGN::setEvent(std::string event) {
	this->event = event;
}
void PGN::setWhiteName(std::string name) {
	this->whiteName = name;
}
void PGN::setBlackName(std::string name) {
	this->blackName = name;
}

void PGN::addMove(std::string move) {
	if (numMoves % 2 == 0) {
		moves << numMoves / 2 << ". ";
	}
	moves << move << " ";
	numMoves++;
}

void PGN::writePGN(std::string path, std::string name) {
	std::ofstream pgnFile;

	if (name.empty()) {
		std::time_t now = std::time(nullptr);
		tm* localTime = localtime(&now);
		std::stringstream ss;
		setDate(1900 + localTime->tm_year, 1 + localTime->tm_mon, localTime->tm_mday);
		ss << path << "log_" << year << "-" << month << "-" << day << "_" <<
			localTime->tm_hour << "-" << localTime->tm_min << "-" << localTime->tm_sec << ".pgn";

		pgnFile.open(ss.str());
	}
	else {
		pgnFile.open(name);
	}

	pgnFile << "[Event \"" << event << "\"]" << std::endl;
	pgnFile << "[Date \"" << year << ".";
	if (month < 10) {
		pgnFile << "0";
	}
	pgnFile << month << ".";
	if (day < 10) {
		pgnFile << "0";
	}
	pgnFile << day << "\"]" << std::endl;
	pgnFile << "[White \"" << whiteName << "\"]" << std::endl;
	pgnFile << "[Black \"" << blackName << "\"]" << std::endl;
	pgnFile << "[Result \"" << result << "\"]" << std::endl;

	pgnFile << std::endl << moves.str();

	if (!pgnFile.is_open()) {
		std::cout << "PGN file could not be generated!" << std::endl;
		return;
	}

	pgnFile.close();
}