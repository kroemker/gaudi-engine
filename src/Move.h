#pragma once

#include <vector>
#include <string>
#include "types.h"
#include "Color.h"
#include "Piece.h"
#include "CastlingRights.h"

class Board;

class Move
{
public:
	enum CastlingMove {
		Kingside,
		Queenside,
		None
	};

	Move() = default;
	// normal move
	Move(int color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights);
	// castling move
	Move(int color, CastlingMove castlingMove, Piece * king, Piece * rook, int kingDest, int rookDest, int oldEnpassantSquare, Piece * oldEnpassantPiece, CastlingRights oldCastlingRights);
	// promotion move
	Move(int color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights, Piece::PieceType promotionType);
	// double pawn move
	Move(int color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int enpassantSquare, Piece* enpassantPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights);

	u8 color;
	u8 source;
	u8 destination;

	u8 enpassantSquare;
	u8 oldEnpassantSquare;

	u8 castleRookSource;
	u8 castleRookDestination;

	Piece* movingPiece;
	Piece* capturedPiece;

	Piece* enpassantPiece;
	Piece* oldEnpassantPiece;

	CastlingMove castlingMove;
	CastlingRights oldCastlingRights;

	Piece::PieceType promotionType;

	std::string toString();
	bool equals(Move& other);
};