#pragma once

class Color {
public:
	static const int WHITE = 0;
	static const int BLACK = 1;
	static const int invert(const int color) {
		return color ^ 1;
	}
};