#include "ClockHandler.h"
#include "Color.h"

#include <algorithm>

ClockHandler::ClockHandler(Board* board) {
	this->board = board;
}

int ClockHandler::getSearchTime(int color) {
	if (type == NORMAL) {
		if (increment[color] == 0) {
			int diff = remaining[color] - remaining[Color::invert(color)];
			// more time than opponent
			if (diff > 0) {
				return std::min(std::max(1000, diff), remaining[color] / 30);
			}
			// less time than opponent
			else {
				return remaining[color] / (30 + (-diff) / 1000);
			}
		}
		else {
			if (board->getNumberOfMoves() < 5) {
				return remaining[color] <= 60000 ? remaining[color] / 60 : 5000;
			}
			if (remaining[color] < 15000) {
				return remaining[color] / 10;
			}
			else if (remaining[color] < 60000) {
				return remaining[color] / 30;
			}
			else if (remaining[color] < 300000) {
				return remaining[color] / 15;
			}
			else if (remaining[color] < 600000) {
				return remaining[color] / 10;
			}
			else {
				return 90000 + std::max(0, increment[color] - 1000);
			}
		}
	}
	else {
		return moveTime;
	}
	return type == NORMAL ? remaining[color] / 30 : moveTime;
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