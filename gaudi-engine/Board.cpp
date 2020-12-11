#include "Board.h"

#include <iostream>
#include <sstream>
#include <cctype>

#include "CastlingRights.h"
#include "Piece.h"

Board::Board() {
	pieceListHolder[0] = &whitePieces;
	pieceListHolder[1] = &blackPieces;

	for (int i = 0; i < 128; i++) {
		board[i] = nullptr;
	}

	enpassantSquare = 128;
	enpassantPiece = nullptr;

	castlingRightsHolder[0] = &whiteCastlingRights;
	castlingRightsHolder[1] = &blackCastlingRights;

	colorToMove = Board::WHITE;
}
Board::~Board() {
	for (int i = 0; i < whitePieces.size(); i++) {
		delete whitePieces[i];
	}
	for (int i = 0; i < blackPieces.size(); i++) {
		delete blackPieces[i];
	}
}

void Board::loadFEN(std::string fen) {
	int len = fen.length();
	int rank = 7;
	int file = 0;
	bool piece = false;

	memset(pieceCount, 0, sizeof(int) * 12);
	// white/black, queenside/kingside
	int rookSquares[2][2] = { { 0, 7 }, { 112, 119 } };

	whitePieces.clear();
	blackPieces.clear();

	int i = 0;
	for (i = 0; i < len; i++) {
		char c = fen[i];
		if (isdigit(c)) {
			file += c - '0';
		}
		else {
			int color = c >= 'a' ? Board::BLACK : Board::WHITE;
			int sq = rank * 16 + file;
			Piece* piece = nullptr;
			switch (tolower(c)) {
			case 'r':
				if (sq == rookSquares[color][0]) {
					piece = new Piece(color, Piece::Rook, sq, false, true);
				}
				else if (sq == rookSquares[color][1]) {
					piece = new Piece(color, Piece::Rook, sq, true, false);
				}
				else {
					piece = new Piece(color, Piece::Rook, sq, false, false);
				}
				break;
			case 'p':
				piece = new Piece(color, Piece::Pawn, sq, false, false);
				break;
			case 'n':
				piece = new Piece(color, Piece::Knight, sq, false, false);
				break;
			case 'k':
				piece = new Piece(color, Piece::King, sq, false, false);
				kings[color] = piece;
				break;
			case 'q':
				piece = new Piece(color, Piece::Queen, sq, false, false);
				break;
			case 'b':
				piece = new Piece(color, Piece::Bishop, sq, false, false);
				break;
			}
			if (piece != nullptr) {
				pieceListHolder[color]->push_back(piece);
				pieceCount[color][piece->type]++;
				file++;
			}
		}
		if (c == '/') {
			rank--;
			file = 0;
		}
		if (c == ' ') {
			break;
		}
	}

	i++;
	if (i < len) {
		if (fen[i] == 'w') {
			colorToMove = Board::WHITE;
		}
		else if (fen[i] == 'b') {
			colorToMove = Board::BLACK;
		}
	}
	i++;
	i++;
	while ((i < len) && (fen[i] != ' ')) {
		int color = fen[i] >= 'a' ? Board::BLACK : Board::WHITE;
		if (tolower(fen[i]) == 'k') {
			castlingRightsHolder[color]->canKingside = true;
		}
		else if (tolower(fen[i]) == 'q') {
			castlingRightsHolder[color]->canQueenside = true;
		}
		i++;
	}
	i++;

	int enpassantFile = -1;
	int enpassantRank = -1;

	if (fen[i] != '-') {
		enpassantFile = fen[i] - 'a';
	}
	i++;
	i++;

	if (enpassantFile >= 0) {
		enpassantRank = fen[i] - '1';
	}

	refillBoardByPieceList();

	if (enpassantFile >= 0) {
		enpassantSquare = enpassantFile + enpassantRank * 16;
		enpassantPiece = enpassantRank == 3 ? board[enpassantSquare + 16] : board[enpassantRank - 16];
	}
}

void Board::refillBoardByPieceList() {
	for (int i = 0; i < 128; i++) {
		board[i] = nullptr;
		for (int j = 0; j < whitePieces.size(); j++) {
			if (whitePieces[j]->square == i) {
				board[i] = whitePieces[j];
			}
		}
		for (int j = 0; j < blackPieces.size(); j++) {
			if (blackPieces[j]->square == i) {
				board[i] = blackPieces[j];
			}
		}
	}
}

bool Board::isMate(Color color) {
	if (!inCheck(color)) {
		return false;
	}

	std::vector<Move> moves;
	generateMoves(color, moves);
	for (int i = 0; i < moves.size(); i++) {
		makeMove(moves[i]);
		if (!inCheck(color)) {
			unmakeMove(moves[i]);
			return false;
		}
		unmakeMove(moves[i]);
	}
	return true;
}

bool Board::inCheck(Color color) {
	return isAttackedBy(getKing(color)->square, invertColor(color));
}

bool Board::sufficientMaterial() {
	for (int c = 0; c < 2; c++) {
		if (pieceCount[c][Piece::Rook] > 0 || pieceCount[c][Piece::Queen] > 0 || pieceCount[c][Piece::Pawn] > 0 ||
			pieceCount[c][Piece::Bishop] > 1 || (pieceCount[c][Piece::Bishop] > 0 && pieceCount[c][Piece::Knight] > 0))
			return true;
	}
	return false;
}

bool Board::isLegalMove(Move& move) {
	std::vector<Move> moves;
	generateMoves(move.color, moves);
	for (int i = 0; i < moves.size(); i++) {
		if (moves[i].equals(move))
			return true;
	}
	return false;
}

bool Board::isAttackedBy(int square, Color color) {
	std::vector<Piece*>* pieces = getPieceList(color);

	for (int i = 0; i < pieces->size(); i++) {
		Piece* current = (*pieces)[i];
		if (!current->alive) {
			continue;
		}

		for (int j = 0; j < current->vectorMoves.size(); j++) {
			int dest = current->square;
			do {
				dest += current->vectorMoves[j];

				// make sure we don't move off the board
				if (!isSquareOnBoard(dest)) {
					break;
				}

				// special rules for pawns
				if (current->type == Piece::Pawn) {
					dest = current->square + current->vectorMoves[j] - 1;
					if (dest == square) {
						return true;
					}

					dest = current->square + current->vectorMoves[j] + 1;
					if (dest == square) {
						return true;
					}

					break;
				}

				if (dest == square) {
					return true;
				}
			} while (current->sliding && isEmptySquare(dest));
		}
	}
	return false;
}

void Board::generateCaptures(Color color, std::vector<Move>& captures) {
	CastlingRights* cr = getCastlingRights(color);
	std::vector<Piece*>* pieces = getPieceList(color);

	for (int i = 0; i < pieces->size(); i++) {
		Piece* current = (*pieces)[i];
		if (!current->alive) {
			continue;
		}

		for (int j = 0; j < current->vectorMoves.size(); j++) {
			int dest = current->square;
			do {
				dest += current->vectorMoves[j];

				// make sure we don't move off the board
				if (!isSquareOnBoard(dest)) {
					break;
				}

				// special rules for pawns
				if (current->type == Piece::Pawn) {
					for (int sq = -1; sq <= 1; sq += 2) {
						dest = current->square + current->vectorMoves[j] + sq;
						if (isSquareOnBoard(dest)) {
							if (!isEmptySquare(dest) && (board[dest]->color != color)) {
								// check promotion
								if (dest >> 4 == 0 || dest >> 4 == 7) {
									captures.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Queen));
									captures.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Knight));
									captures.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Rook));
									captures.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Bishop));
								}
								else {
									captures.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr));
								}
							}
							else if (dest == enpassantSquare && enpassantPiece->color != color) {
								captures.push_back(Move(color, current->square, dest, current, enpassantPiece, enpassantSquare, enpassantPiece, *cr));
							}
						}
					}
					break;
				}

				// look if capture a piece
				if (!isEmptySquare(dest) && (board[dest]->color != color)) {
					captures.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr));
				}
			} while (current->sliding && isEmptySquare(dest));
		}
	}
}

void Board::generateMoves(Color color, std::vector<Move>& moves) {
	CastlingRights* cr = getCastlingRights(color);

	bool incheck = inCheck(color);

	std::vector<Piece*>* pieces = getPieceList(color);

	for (int i = 0; i < pieces->size(); i++) {
		Piece* current = (*pieces)[i];
		if (!current->alive) {
			continue;
		}

		// castling
		if (current->type == Piece::King && !incheck) {
			int dest = current->square + 2;
			if (cr->canKingside && isEmptySquare(dest - 1) && isEmptySquare(dest) && !isAttackedBy(dest, invertColor(color)) && !isAttackedBy(dest - 1, invertColor(color))) {
				moves.push_back(Move(color, Move::Kingside, current, board[dest + 1], enpassantSquare, enpassantPiece, *cr));
			}

			int dest = current->square - 2;
			if (cr->canQueenside && isEmptySquare(dest - 1) && isEmptySquare(dest) && isEmptySquare(dest + 1) && !isAttackedBy(dest, invertColor(color)) && !isAttackedBy(dest + 1, invertColor(color))) {
				moves.push_back(Move(color, Move::Queenside, current, board[dest - 2], enpassantSquare, enpassantPiece, *cr));
			}
		}

		for (int j = 0; j < current->vectorMoves.size(); j++) {
			int dest = current->square;
			do {
				dest += current->vectorMoves[j];

				// make sure we don't move off the board
				if (!isSquareOnBoard(dest)) {
					break;
				}

				// special rules for pawns
				if (current->type == Piece::Pawn) {
					// move forward
					if (isEmptySquare(dest)) {
						// check promotion
						if (dest >> 4 == 0 || dest >> 4 == 7) {
							moves.push_back(Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, *cr, Piece::Queen));
							moves.push_back(Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, *cr, Piece::Knight));
							moves.push_back(Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, *cr, Piece::Rook));
							moves.push_back(Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, *cr, Piece::Bishop));
						}
						else {
							moves.push_back(Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, *cr));
						}

						// double move
						if ((current->square >> 4 == 1 && color == WHITE) || (current->square >> 4 == 6 && color == BLACK)) {
							dest += current->vectorMoves[j];
							if (isSquareOnBoard(dest) && isEmptySquare(dest)) {
								moves.push_back(Move(color, current->square, dest, current, nullptr, dest - current->vectorMoves[j], current, enpassantSquare, enpassantPiece, *cr));
							}
						}
					}

					// capture moves
					for (int sq = -1; sq <= 1; sq += 2) {
						dest = current->square + current->vectorMoves[j] + sq;
						if (isSquareOnBoard(dest)) {
							if ((!isEmptySquare(dest)) && (board[dest]->color != color)) {
								// check promotion
								if (dest >> 4 == 0 || dest >> 4 == 7) {
									moves.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Queen));
									moves.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Knight));
									moves.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Rook));
									moves.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr, Piece::Bishop));
								}
								else {
									moves.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr));
								}
							}
							else if (dest == enpassantSquare && enpassantPiece->color != color) {
								moves.push_back(Move(color, current->square, dest, current, enpassantPiece, enpassantSquare, enpassantPiece, *cr));
							}
						}
					}
					break;
				}

				if (!isEmptySquare(dest) && board[dest]->color == color) {
					break;
				}

				moves.push_back(Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, *cr));

			} while (current->sliding && isEmptySquare(dest));
		}
	}
}
void Board::makeMove(Move& move) {
	// update en passant state
	enpassantSquare = move.enpassantSquare;
	enpassantPiece = move.enpassantPiece;

	// castling
	if (move.castlingMove == Move::Kingside) {
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square += 2;
		move.capturedPiece->square -= 2;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
		getCastlingRights(move.color)->unsetAll();
		colorToMove = invertColor(colorToMove);
		return;
	}
	else if (move.castlingMove == Move::Queenside) {
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square -= 2;
		move.capturedPiece->square += 3;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
		getCastlingRights(move.color)->unsetAll();
		colorToMove = invertColor(colorToMove);
		return;
	}

	// update piece capture
	if (move.capturedPiece != nullptr) {
		move.capturedPiece->alive = false;
		board[move.capturedPiece->square] = nullptr;
		pieceCount[move.capturedPiece->color][move.capturedPiece->type]--;

		if (move.capturedPiece->isKingsideRook) {
			getCastlingRights(invertColor(move.color))->canKingside = false;
		}
		else if (move.capturedPiece->isQueensideRook) {
			getCastlingRights(invertColor(move.color))->canQueenside = false;
		}
	}

	move.movingPiece->square = move.destination;

	// check promotion
	if (move.promotionType != Piece::None) {
		move.movingPiece->type = move.promotionType;
		move.movingPiece->sliding = Piece::isSliding(move.promotionType);
		move.movingPiece->setupVectorMoves();
	}

	// unset castling rights
	if (move.movingPiece->type == Piece::King) {
		getCastlingRights(invertColor(move.color))->unsetAll();
	}
	else if (move.movingPiece->isKingsideRook) {
		getCastlingRights(invertColor(move.color))->canKingside = false;
	}
	else if (move.movingPiece->isQueensideRook) {
		getCastlingRights(invertColor(move.color))->canQueenside = false;
	}

	// update board
	board[move.destination] = move.movingPiece;
	board[move.source] = nullptr;

	colorToMove = invertColor(colorToMove);
}
void Board::unmakeMove(Move& move) {
	//en passant
	enpassantSquare = move.oldEnpassantSquare;
	enpassantPiece = move.oldEnpassantPiece;

	//castling rights
	(*getCastlingRights(move.color)) = move.oldCastlingRights;

	//update castling
	if (move.castlingMove == Move::Kingside)
	{
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square -= 2;
		move.capturedPiece->square += 2;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
		colorToMove = invertColor(colorToMove);
		return;
	}
	else if (move.castlingMove == Move::Queenside)
	{
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square += 2;
		move.capturedPiece->square -= 3;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
		colorToMove = invertColor(colorToMove);
		return;
	}


	board[move.destination] = nullptr; // needed for enpassant capture

	//update piece list
	if (move.capturedPiece != nullptr)
	{
		move.capturedPiece->alive = true;
		board[move.capturedPiece->square] = move.capturedPiece;
		pieceCount[move.capturedPiece->color][move.capturedPiece->type]++;
	}

	move.movingPiece->square = move.source;

	if (move.promotionType != Piece::None)
	{
		move.movingPiece->type = Piece::Pawn;
		move.movingPiece->sliding = false;
		move.movingPiece->setupVectorMoves();
	}

	//update board
	board[move.source] = move.movingPiece;

	colorToMove = invertColor(colorToMove);
}

void Board::print(std::ostringstream& out) {
	int pos = 112;
	while (pos >= 0)
	{
		for (int i = 0; i < 8; i++)
		{
			if (!isEmptySquare(pos + i)) {
				out << board[pos + i]->toString();
			}
			else {
				out << "--";
			}
			out << " ";
		}
		out << std::endl;
		pos -= 16;
	}
}

// getters
int Board::getColorToMove() {
	return colorToMove;
}
void Board::setColorToMove(Color c) {
	colorToMove = c;
}
Piece* Board::getPiece(int pos) {
	return board[pos];
}
Piece* Board::getKing(Color color) {
	return kings[color];
}
int Board::getEnpassantSquare() {
	return enpassantSquare;
}
Piece* Board::getEnpassantPiece() {
	return enpassantPiece;
}
CastlingRights* Board::getCastlingRights(Color color) {
	return castlingRightsHolder[color];
}
std::vector<Piece*>* Board::getPieceList(Color color) {
	return pieceListHolder[color];
}
bool Board::isEmptySquare(int square) {
	return board[square] == nullptr;
}
bool Board::isSquareOnBoard(int square) {
	return (square & 0x88) == 0;
}

std::string Board::getMoveStringAlgebraic(Move& move) {

}

char Board::getFileBySquare(int square) {
	return (square & 7) + 'a';
}
char Board::getRankBySquare(int square) {
	return (square >> 4) + '1';
}

char Board::getCharOfPiece(Piece::PieceType type) {
	switch (type) {
	case Piece::Bishop:
		return 'B';
	case Piece::Rook:
		return 'R';
	case Piece::Knight:
		return 'N';
	case Piece::Pawn:
		return 'P';
	case Piece::King:
		return 'K';
	case Piece::Queen:
		return 'Q';
	default:
		return 'E';
	}
}

inline Board::Color Board::invertColor(Color c) {
	return (Color)(c ^ 1);
}