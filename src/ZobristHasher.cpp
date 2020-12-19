#include "ZobristHasher.h"
#include "Color.h"
#include "Board.h"
#include "Piece.h"

#include <cstdlib>
#include <ctime>

ZobristHasher::ZobristHasher(Board* board) {
	this->board = board;

	//srand(time(NULL));

	for (int i = 0; i < tableSize; i++) {
		table[i] = (u64)rand() << 32 | (u64)rand();
	}
}

void ZobristHasher::hashNew() {
	hash = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			Piece* piece = board->getPiece(i + 16 * j);
			if (piece != nullptr && piece->alive) {
				int offset = piece->color * 64 * 6;
				int square = j * 8 + i;
				int type = piece->type;
				hash ^= table[offset + square + type * 64];
			}
		}
	}

	castlingRightIndex = board->getCastlingRights().getRaw();
	hash ^= table[2 * 64 * 6 + castlingRightIndex];

	if (board->getEnpassantSquare() < 128) {
		hash ^= table[2 * 64 * 6 + 16 + board->getEnpassantSquare() & 7];
	}

	if (board->getColorToMove() == Color::BLACK)
		hash ^= table[tableSize - 1];

}

void ZobristHasher::updateHash(Move& m) {
	int offset = m.color * 64 * 6;
	if (m.castlingMove == Move::None)
	{
		int dsquare = Board::convert88To64Square(m.destination);
		int ssquare = Board::convert88To64Square(m.source);

		if (m.promotionType == Piece::None) {
			int type = m.movingPiece->type;
			hash ^= table[offset + ssquare + type * 64]; // remove piece from src
			hash ^= table[offset + dsquare + type * 64]; // place piece on dest
		}
		else {
			hash ^= table[offset + ssquare + Piece::Pawn * 64]; // remove pawn piece from src
			hash ^= table[offset + dsquare + m.promotionType * 64]; // place promo piece on dest
		}

		if (m.capture)
		{
			offset = m.capturedPiece->color * 64 * 6;
			int type = m.capturedPiece->type;
			dsquare = Board::convert88To64Square(m.capturedPiece->square); // need this in case of enpassant capture
			hash ^= table[offset + dsquare + type * 64]; // remove captured piece from dst
		}
	}
	else
	{
		int kingdsquare = Board::convert88To64Square(m.destination);
		int rookdsquare = Board::convert88To64Square(m.castleRookDestination);
		int kingssquare = Board::convert88To64Square(m.source);
		int rookssquare = Board::convert88To64Square(m.castleRookSource);
		// put pieces
		hash ^= table[offset + kingdsquare + Piece::King * 64];
		hash ^= table[offset + rookdsquare + Piece::Rook * 64];
		// remove pieces
		hash ^= table[offset + kingssquare + Piece::King * 64];
		hash ^= table[offset + rookssquare + Piece::Rook * 64];
	}

	hash ^= table[2 * 64 * 6 + castlingRightIndex]; // undo old castling rights
	castlingRightIndex = board->getCastlingRights().getRaw();
	hash ^= table[2 * 64 * 6 + castlingRightIndex];

	// enpassant square
	if (m.enpassantSquare < 128) {
		hash ^= table[2 * 64 * 6 + 16 + m.enpassantSquare & 7];
	}
	if (m.oldEnpassantSquare < 128) {
		hash ^= table[2 * 64 * 6 + 16 + m.oldEnpassantSquare & 7];
	}

	// switch side to move
	hash ^= table[tableSize - 1];
}

u64 ZobristHasher::getHash() {
	return hash;
}