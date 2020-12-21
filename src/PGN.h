
#include <sstream>
#include "Move.h"

class PGN
{
public:
	PGN();

	void setDate(int year, int month, int day);
	void setResult(std::string result);
	void setEvent(std::string event);
	void setWhiteName(std::string name);
	void setBlackName(std::string name);
	void addMove(std::string move);
	void writePGN(std::string name = "");
private:
	std::string event;
	std::string result;
	std::string whiteName;
	std::string blackName;

	int year;
	int month;
	int day;
	std::stringstream moves;

	int numMoves;
};