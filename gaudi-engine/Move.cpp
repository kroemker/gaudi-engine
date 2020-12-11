#include "Move.h"

#include "Board.h"

// normal move
Move::Move(Board::Color color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights) {
	this->color = color;
	this->source = source;
	this->destination = destination;
	this->movingPiece = movingPiece;
	this->capturedPiece = capturedPiece;
	this->enpassantSquare = 128;
	this->enpassantPiece = nullptr;
	this->oldEnpassantSquare = oldEnpassantSquare;
	this->oldEnpassantPiece = oldEnpassantPiece;
	this->oldCastlingRights = oldCastlingRights;
	this->castlingMove = CastlingMove::None;
	this->promotionType = Piece::None;
	this->capture = capturedPiece != nullptr;
}
// castling move
Move::Move(Board::Color color, CastlingMove castlingMove, Piece* king, Piece* rook, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights) {
	this->color = color;
	this->castlingMove = castlingMove;
	this->source = source;
	this->destination = 128;
	this->movingPiece = king;
	this->capturedPiece = rook;
	this->rookSrc = rook->square;
	this->kingSrc = king->square;
	this->enpassantSquare = 128;
	this->enpassantPiece = nullptr;
	this->oldEnpassantSquare = oldEnpassantSquare;
	this->oldEnpassantPiece = oldEnpassantPiece;
	this->oldCastlingRights = oldCastlingRights;
	this->promotionType = Piece::None;
}
// promotion move
Move::Move(Board::Color color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights, Piece::PieceType promotionType) {
	this->color = color;
	this->source = source;
	this->destination = destination;
	this->movingPiece = movingPiece;
	this->capturedPiece = capturedPiece;
	this->enpassantSquare = 128;
	this->enpassantPiece = nullptr;
	this->oldEnpassantSquare = oldEnpassantSquare;
	this->oldEnpassantPiece = oldEnpassantPiece;
	this->oldCastlingRights = oldCastlingRights;
	this->castlingMove = CastlingMove::None;
	this->promotionType = promotionType;
	this->capture = capturedPiece != nullptr;

}
// double pawn move
Move::Move(Board::Color color, int source, int destination, Piece* movingPiece, Piece* capturedPiece, int enpassantSquare, Piece* enpassantPiece, int oldEnpassantSquare, Piece* oldEnpassantPiece, CastlingRights oldCastlingRights) {
	this->color = color;
	this->source = source;
	this->destination = destination;
	this->movingPiece = movingPiece;
	this->capturedPiece = capturedPiece;
	this->enpassantSquare = enpassantSquare;
	this->enpassantPiece = enpassantPiece;
	this->oldEnpassantSquare = oldEnpassantSquare;
	this->oldEnpassantPiece = oldEnpassantPiece;
	this->oldCastlingRights = oldCastlingRights;
	this->castlingMove = CastlingMove::None;
	this->promotionType = promotionType;
	this->capture = capturedPiece != nullptr;
}

std::string Move::toString() {
	if (castlingMove == Kingside && color == Board::BLACK)
		return "e8g8";
	else if (castlingMove == Kingside && color == Board::WHITE)
		return "e1g1";
	else if (castlingMove == Queenside && color == Board::BLACK)
		return "e8c8";
	else if (castlingMove == Queenside && color == Board::WHITE)
		return "e1c1";
	else if (promotionType != Piece::None) {
		char buff[32];
		sprintf(buff, "%c%c%c%c%c", Board::getFileBySquare(source), Board::getRankBySquare(source), Board::getFileBySquare(destination), Board::getRankBySquare(destination), Board::getCharOfPiece(promotionType));
		return std::string(buff);
	}
	else {
		char buff[32];
		sprintf(buff, "%c%c%c%c", Board::getFileBySquare(source), Board::getRankBySquare(source), Board::getFileBySquare(destination), Board::getRankBySquare(destination));
		return std::string(buff);
	}
}

bool Move::equals(Move& move) {
	return (color == move.color && source == move.source && destination == move.destination);
}