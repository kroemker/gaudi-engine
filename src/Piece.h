#pragma once
#include <vector>
#include <string>
#include "types.h"

class Piece
{
public:
	enum PieceType : u8 {
		King,
		Pawn,
		Knight,
		Bishop,
		Rook,
		Queen,
		None
	};

	Piece(int color, PieceType type, int square, bool isKingsideRook, bool isQueensideRook);
	~Piece();

	std::string toString();
	void setupVectorMoves();

	static bool isSliding(PieceType type);

	int color;
	PieceType type;
	int square;
	bool sliding;
	bool hasMoved;
	bool isKingsideRook;
	bool isQueensideRook;
	int* vectorMoves;
	int numVectorMoves;
	bool alive;
};

