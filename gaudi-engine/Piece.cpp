#include "Piece.h"
#include "Board.h"

static int sWhitePawnVectorTable[] = { 16 };
static int sBlackPawnVectorTable[] = { -16 };
static int sKnightVectorTable[] = { 14,18,-14,-18,31,33,-31,-33 };
static int sKingVectorTable[] = { 1,17,16,15,-1,-17,-16,-15 };
static int sBishopVectorTable[] = { 17,15,-17,-15 };
static int sRookVectorTable[] = { 16,1,-16,-1 };
static int sQueenVectorTable[] = { 1,17,16,15,-1,-17,-16,-15 };

Piece::Piece(int color, PieceType type, int square, bool isKingsideRook=false, bool isQueensideRook=false) {
	this->color = color;
	this->type = type;
	this->square = square;
	this->sliding = isSliding(type);
	this->hasMoved = false;
	this->isKingsideRook = isKingsideRook;
	this->isQueensideRook = isQueensideRook;
	this->alive = true;
	setupVectorMoves();
}

Piece::~Piece() {

}

std::string Piece::toString() {
	std::string out = "";
	switch (type) {
	case Pawn:
		out.append("P");
		break;
	case Knight:
		out.append("N");
		break;
	case King:
		out.append("K");
		break;
	case Bishop:
		out.append("B");
		break;
	case Rook:
		out.append("R");
		break;
	case Queen:
		out.append("Q");
		break;
	default:
		return "";
	}

	if (color == Board::WHITE) {
		out.append("w");
	}
	else {
		out.append("b");
	}
	return out;
}

bool Piece::isSliding(PieceType type) {
	return type == Bishop || type == Rook || type == Queen;
}

void Piece::setupVectorMoves() {
	int* ptr = nullptr;
	int num = 0;
	switch (type) {
	case Pawn:
		ptr = color == Board::WHITE ? sWhitePawnVectorTable : sBlackPawnVectorTable;
		num = 1;
		break;
	case Knight:
		ptr = sKnightVectorTable;
		num = 8;
		break;
	case King:
		ptr = sKingVectorTable;
		num = 8;
		break;
	case Bishop:
		ptr = sBishopVectorTable;
		num = 4;
		break;
	case Rook:
		ptr = sRookVectorTable;
		num = 4;
		break;
	case Queen:
		ptr = sKnightVectorTable;
		num = 8;
		break;
	default:
	}
	for (int i = 0; i < num; i++) {
		vectorMoves.push_back(ptr[i]);
	}
}