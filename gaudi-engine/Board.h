#pragma once

#include "CastlingRights.h"
#include "Move.h"

class Board
{
public:
	enum Color {
		WHITE,
		BLACK
	};

	Board();
	~Board();

	void loadFEN(std::string fen);
	void refillBoardByPieceList();

	bool isMate(Color color);
	bool inCheck(Color color);
	bool sufficientMaterial();
	bool isLegalMove(Move& move);
	bool isAttackedBy(int square, Color color);

	void generateCaptures(Color color, std::vector<Move>& captures);
	void generateMoves(Color color, std::vector<Move>& moves);
	void makeMove(Move& move);
	void unmakeMove(Move& move);

	void print(std::ostringstream& out);

	// getters
	int getColorToMove();
	void setColorToMove(Color c);
	Piece* getPiece(int pos);
	Piece* getKing(Color color);
	int getEnpassantSquare();
	Piece* getEnpassantPiece();
	CastlingRights* getCastlingRights(Color color);
	std::vector<Piece*>* getPieceList(Color color);
	bool isEmptySquare(int square);
	bool isSquareOnBoard(int square);

	std::string getMoveStringAlgebraic(Move& move);

	static char getFileBySquare(int square);
	static char getRankBySquare(int square);
	static char getCharOfPiece(Piece::PieceType type);
	static inline Color invertColor(Color c);
private:
	Piece* board[128];
	int pieceCount[2][6];

	Color colorToMove;

	int enpassantSquare;
	Piece* enpassantPiece;

	CastlingRights whiteCastlingRights;
	CastlingRights blackCastlingRights;

	Piece* kings[2];

	std::vector<Piece*> whitePieces;
	std::vector<Piece*> blackPieces;

	std::vector<Piece*>* pieceListHolder[2];
	CastlingRights* castlingRightsHolder[2];
};

