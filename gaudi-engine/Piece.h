#pragma once
#include <vector>

class Piece
{
public:
	enum PieceType {
		Pawn,
		Knight,
		King,
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
	std::vector<int> vectorMoves;
	bool alive;

};

