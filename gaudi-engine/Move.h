#pragma once

#include "Board.h"
#include "Piece.h"
#include "CastlingRights.h"

class Move
{
public:
	enum CastlingMove {
		Kingside,
		Queenside,
		None
	};

	// normal move
	Move(Board::Color color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights);
	// castling move
	Move(Board::Color color, CastlingMove castlingMove, Piece* king, Piece* rook, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights);
	// promotion move
	Move(Board::Color color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights, Piece::PieceType promotionType);
	// double pawn move
	Move(Board::Color color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int enpassantSquare, Piece* enpassantPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights);

	Board::Color color;
	int source;
	int destination;
	Piece* movingPiece;
	Piece* capturedPiece;
	bool capture;

	int enpassantSquare;
	Piece* enpassantPiece;
	int oldEnpassantSquare;
	Piece* oldEnpassantPiece;

	CastlingMove castlingMove;
	CastlingRights oldCastlingRights;

	int rookSrc;
	int kingSrc;

	Piece::PieceType promotionType;

	std::string toString();
	bool equals(Move& other);
};

