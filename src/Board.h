#pragma once

#include <vector>
#include <string>
#include "CastlingRights.h"
#include "Move.h"
#include "Piece.h"
#include "Color.h"
#include "ZobristHasher.h"
#include "AttackMap.h"
#include "hashing/HashTable.h"
#include "hashing/AttackMapEntry.h"

class Board
{
public:
	Board();
	~Board();

	void loadFEN(std::string fen);
	void loadStartPosition();
	void refillBoardByPieceList();

	bool isMate(int color);
	bool isStalemate(int color);
	bool inCheck(int color);
	bool sufficientMaterial();
	bool isRepetition();
	bool isLegalMove(Move& move);
	bool isAttackedBy(int square, int color);

	int generateCaptures(Move * captures);
	int generateCaptures(int color, Move* captures);
	int generateMoves(Move* moves);
	int generateMoves(int color, Move* moves);
	void makeMove(Move& move);
	void unmakeMove(Move& move);

	void print(std::ostream& out);
	void cleanupDeadPieces();

	// getters
	int getColorToMove();
	void setColorToMove(int c);
	Piece* getPiece(int pos);
	Piece* getKing(int color);
	int getEnpassantSquare();
	Piece* getEnpassantPiece();
	CastlingRights getCastlingRights();
	std::vector<Piece*>* getPieceList(int color); 
	int getPieceCount(int color, Piece::PieceType type);
	int getTotalPieceCount(Piece::PieceType type);
	u64 getHash();
	int getNumberOfMoves();
	bool isEmptySquare(int square);
	bool isSquareOnBoard(int square);

	std::string getMoveStringAlgebraic(Move & move, bool requireUpperCasePromotionType = false);

	static char getFileBySquare(int square);
	static char getRankBySquare(int square);
	int getSquareFromString(std::string str);
	static char getCharOfPiece(Piece::PieceType type);
	static Piece::PieceType getPieceTypeFromChar(char type);
	static int convert88To64Square(int square0x88);
	static int convert64To88Square(int square64);
private:
	Piece* board[128];
	int pieceCount[2][6];

	int colorToMove;
	AttackMap attackMap;

	int enpassantSquare;
	Piece* enpassantPiece;
	CastlingRights castlingRights;

	Piece* kings[2];

	std::vector<Piece*> whitePieces;
	std::vector<Piece*> blackPieces;

	std::vector<Piece*>* pieceListHolder[2];

	ZobristHasher zobristHasher;
	std::vector<u64> hashHistory;
	std::vector<std::string> moveStringHistory;
};

