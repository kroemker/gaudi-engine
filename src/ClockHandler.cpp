#include "ClockHandler.h"

int ClockHandler::getSearchTime(int color) {
	return type == NORMAL ? increment[color] / 2 + remaining[color] / 30 : moveTime;
}

void ClockHandler::setClockTime(int color, int timeMs) {
	remaining[color] = timeMs;
	type = NORMAL;
}

void ClockHandler::setClockIncrement(int color, int timeMs) {
	increment[color] = timeMs;
	type = NORMAL;
}

void ClockHandler::setMoveTime(int timeMs) {
	moveTime = timeMs;
	type = MOVETIME;
}