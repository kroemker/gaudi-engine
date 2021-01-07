#pragma once
#include "types.h"
#include "Color.h"

class CastlingRights
{
public:

	CastlingRights(bool whiteCanKingside=true, bool whiteCanQueenside=true, bool blackCanKingside = true, bool blackcanQueenside = true) {
		if (whiteCanKingside) {
			rights |= WHITE_KINGSIDE;
		}
		if (whiteCanQueenside) {
			rights |= WHITE_QUEENSIDE;
		}
		if (blackCanKingside) {
			rights |= BLACK_KINGSIDE;
		}
		if (blackcanQueenside) {
			rights |= BLACK_KINGSIDE;
		}
	}

	void unsetAll() {
		unsetAll(Color::WHITE);
		unsetAll(Color::BLACK);
	}

	void unsetAll(int color) {
		unsetCastleKingside(color);
		unsetCastleQueenside(color);
	}

	bool canCastleKingside(int color) {
		return rights & (KINGSIDE << (color * 2));
	}

	bool canCastleQueenside(int color) {
		return rights & (QUEENSIDE << (color * 2));
	}

	void setCastleKingside(int color) {
		rights |= KINGSIDE << (color * 2);
	}

	void setCastleQueenside(int color) {
		rights |= QUEENSIDE << (color * 2);
	}

	void unsetCastleKingside(int color) {
		rights &= ~(KINGSIDE << (color * 2));
	}

	void unsetCastleQueenside(int color) {
		rights &= ~(QUEENSIDE << (color * 2));
	}

	u8 getRaw() {
		return rights;
	}

	std::string getString() {
		char arr[5];
		arr[0] = canCastleKingside(Color::WHITE) ? 'K' : '-';
		arr[1] = canCastleQueenside(Color::WHITE) ? 'Q' : '-';
		arr[2] = canCastleKingside(Color::BLACK) ? 'k' : '-';
		arr[3] = canCastleQueenside(Color::BLACK) ? 'q' : '-';
		arr[4] = 0;

		return std::string(arr);
	}

private:
	static const u8 KINGSIDE = 0x02;
	static const u8 QUEENSIDE = 0x01;
	static const u8 BLACK_KINGSIDE = KINGSIDE << 2;
	static const u8 BLACK_QUEENSIDE = QUEENSIDE << 2;
	static const u8 WHITE_KINGSIDE = KINGSIDE;
	static const u8 WHITE_QUEENSIDE = QUEENSIDE;

	u8 rights; // 0000 kqKQ
};

