#pragma once
class ClockHandler
{
public:
	enum ClockType {
		MOVETIME,
		NORMAL
	};

	int getSearchTime(int color);
	void setClockTime(int color, int timeMs);
	void setClockIncrement(int color, int timeMs);
	void setMoveTime(int timeMs);

private:
	int remaining[2];
	int increment[2];
	int moveTime;
	int type;
};

