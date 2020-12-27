#include "Board.h"

#include <iostream>
#include <sstream>
#include <cctype>

#include "CastlingRights.h"
#include "Piece.h"

Board::Board() : zobristHasher(this) {
	pieceListHolder[0] = &whitePieces;
	pieceListHolder[1] = &blackPieces;

	for (int i = 0; i < 128; i++) {
		board[i] = nullptr;
	}

	enpassantSquare = 128;
	enpassantPiece = nullptr;

	colorToMove = Color::WHITE;
	attackMap = { 0 };

	hashHistory.reserve(200);
	hashHistory.clear();
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
	size_t len = fen.length();
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
			int color = c >= 'a' ? Color::BLACK : Color::WHITE;
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
			colorToMove = Color::WHITE;
		}
		else if (fen[i] == 'b') {
			colorToMove = Color::BLACK;
		}
	}
	i++;
	i++;
	castlingRights.unsetAll();
	while ((i < len) && (fen[i] != ' ')) {
		int color = fen[i] >= 'a' ? Color::BLACK : Color::WHITE;
		if (tolower(fen[i]) == 'k') {
			castlingRights.setCastleKingside(color);
		}
		else if (tolower(fen[i]) == 'q') {
			castlingRights.setCastleQueenside(color);
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

	zobristHasher.hashNew();
	hashHistory.clear();
	moveStringHistory.clear();
}

void Board::loadStartPosition() {
	loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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

bool Board::isMate(int color) {
	if (!inCheck(color)) {
		return false;
	}

	Move moves[128];
	int n = generateMoves(color, moves);
	for (int i = 0; i < n; i++) {
		makeMove(moves[i]);
		if (!inCheck(color)) {
			unmakeMove(moves[i]);
			return false;
		}
		unmakeMove(moves[i]);
	}
	return true;
}

bool Board::isStalemate(int color)
{
	if (inCheck(color)) {
		return false;
	}

	Move moves[128];
	int n = generateMoves(color, moves);
	for (int i = 0; i < n; i++) {
		makeMove(moves[i]);
		if (inCheck(color)) {
			unmakeMove(moves[i]);
			continue;
		}
		unmakeMove(moves[i]);
		return false;
	}
	return true;
}

bool Board::inCheck(int color) {
	return isAttackedBy(getKing(color)->square, Color::invert(color));
}

bool Board::sufficientMaterial() {
	for (int c = 0; c < 2; c++) {
		if (pieceCount[c][Piece::Rook] > 0 || pieceCount[c][Piece::Queen] > 0 || pieceCount[c][Piece::Pawn] > 0 ||
			pieceCount[c][Piece::Bishop] > 1 || (pieceCount[c][Piece::Bishop] > 0 && pieceCount[c][Piece::Knight] > 0))
			return true;
	}
	return false;
}

bool Board::isRepetition() {
	bool f = false;
	for (int i = 0; i < hashHistory.size(); i++) {
		if (getHash() == hashHistory[i]) {
			if (f) {
				return true;
			}
			else {
				f = true;
			}
		}
	}
	return false;
}

bool Board::isLegalMove(Move& move) {
	Move moves[128];
	int n = generateMoves(move.color, moves);
	for (int i = 0; i < n; i++) {
		if (moves[i].equals(move))
			return true;
	}
	return false;
}

bool Board::isAttackedBy(int square, int color) {
	if (attackMap.map[color] == 0) {
		std::vector<Piece*>* pieces = getPieceList(color);

		for (int i = 0; i < pieces->size(); i++) {
			Piece* current = (*pieces)[i];
			if (!current->alive) {
				continue;
			}

			for (int j = 0; j < current->numVectorMoves; j++) {
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
						if (isSquareOnBoard(dest)) {
							attackMap.map[color] |= ((u64)1 << convert88To64Square(dest));
						}

						dest = current->square + current->vectorMoves[j] + 1;
						if (isSquareOnBoard(dest)) {
							attackMap.map[color] |= ((u64)1 << convert88To64Square(dest));
						}

						break;
					}

					attackMap.map[color] |= ((u64)1 << convert88To64Square(dest));
				} while (current->sliding && isEmptySquare(dest));
			}
		}
	}
	return attackMap.map[color] & ((u64)1 << convert88To64Square(square));
}

int Board::generateCaptures(Move* captures) {
	return generateCaptures(colorToMove, captures);
}

int Board::generateCaptures(int color, Move* captures) {
	int numCaptures = 0;
	std::vector<Piece*>* pieces = getPieceList(color);

	for (int i = 0; i < pieces->size(); i++) {
		Piece* current = (*pieces)[i];
		if (!current->alive) {
			continue;
		}

		for (int j = 0; j < current->numVectorMoves; j++) {
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
									captures[numCaptures++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Queen);
									captures[numCaptures++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Knight);
									captures[numCaptures++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Rook);
									captures[numCaptures++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Bishop);
								}
								else {
									captures[numCaptures++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights);
								}
							}
							else if (dest == enpassantSquare && enpassantPiece->color != color) {
								captures[numCaptures++] = Move(color, current->square, dest, current, enpassantPiece, enpassantSquare, enpassantPiece, castlingRights);
							}
						}
					}
					break;
				}

				// look if capture a piece
				if (!isEmptySquare(dest) && (board[dest]->color != color)) {
					captures[numCaptures++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights);
				}
			} while (current->sliding && isEmptySquare(dest));
		}
	}
	return numCaptures;
}


int Board::generateMoves(Move* moves) {
	return generateMoves(colorToMove, moves);
}

int Board::generateMoves(int color, Move* moves) {
	int numMoves = 0;

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
			if (castlingRights.canCastleKingside(color) && isEmptySquare(dest - 1) && isEmptySquare(dest) && !isAttackedBy(dest, Color::invert(color)) && !isAttackedBy(dest - 1, Color::invert(color))) {
				moves[numMoves++] = Move(color, Move::Kingside, current, board[dest + 1], dest, dest - 1, enpassantSquare, enpassantPiece, castlingRights);
			}

			dest = current->square - 2;
			if (castlingRights.canCastleQueenside(color) && isEmptySquare(dest - 1) && isEmptySquare(dest) && isEmptySquare(dest + 1) && !isAttackedBy(dest, Color::invert(color)) && !isAttackedBy(dest + 1, Color::invert(color))) {
				moves[numMoves++] = Move(color, Move::Queenside, current, board[dest - 2], dest, dest + 1, enpassantSquare, enpassantPiece, castlingRights);
			}
		}

		for (int j = 0; j < current->numVectorMoves; j++) {
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
							moves[numMoves++] = Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, castlingRights, Piece::Queen);
							moves[numMoves++] = Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, castlingRights, Piece::Knight);
							moves[numMoves++] = Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, castlingRights, Piece::Rook);
							moves[numMoves++] = Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, castlingRights, Piece::Bishop);
						}
						else {
							moves[numMoves++] = Move(color, current->square, dest, current, nullptr, enpassantSquare, enpassantPiece, castlingRights);
						}

						// double move
						if ((current->square >> 4 == 1 && color == Color::WHITE) || (current->square >> 4 == 6 && color == Color::BLACK)) {
							dest += current->vectorMoves[j];
							if (isSquareOnBoard(dest) && isEmptySquare(dest)) {
								moves[numMoves++] = Move(color, current->square, dest, current, nullptr, dest - current->vectorMoves[j], current, enpassantSquare, enpassantPiece, castlingRights);
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
									moves[numMoves++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Queen);
									moves[numMoves++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Knight);
									moves[numMoves++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Rook);
									moves[numMoves++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights, Piece::Bishop);
								}
								else {
									moves[numMoves++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights);
								}
							}
							else if (dest == enpassantSquare && enpassantPiece->color != color) {
								moves[numMoves++] = Move(color, current->square, dest, current, enpassantPiece, enpassantSquare, enpassantPiece, castlingRights);
							}
						}
					}
					break;
				}

				if (!isEmptySquare(dest) && board[dest]->color == color) {
					break;
				}

				moves[numMoves++] = Move(color, current->square, dest, current, board[dest], enpassantSquare, enpassantPiece, castlingRights);

			} while (current->sliding && isEmptySquare(dest));
		}
	}
	return numMoves;
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
		castlingRights.unsetAll(move.color);
	}
	else if (move.castlingMove == Move::Queenside) {
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square -= 2;
		move.capturedPiece->square += 3;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
		castlingRights.unsetAll(move.color);
	}
	else {
		// update piece capture
		if (move.capturedPiece != nullptr) {
			move.capturedPiece->alive = false;
			board[move.capturedPiece->square] = nullptr;
			pieceCount[move.capturedPiece->color][move.capturedPiece->type]--;

			if (move.capturedPiece->isKingsideRook) {
				castlingRights.unsetCastleKingside(Color::invert(move.color));
			}
			else if (move.capturedPiece->isQueensideRook) {
				castlingRights.unsetCastleQueenside(Color::invert(move.color));
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
			castlingRights.unsetAll(move.color);
		}
		else if (move.movingPiece->isKingsideRook) {
			castlingRights.unsetCastleKingside(move.color);
		}
		else if (move.movingPiece->isQueensideRook) {
			castlingRights.unsetCastleQueenside(move.color);
		}

		// update board
		board[move.destination] = move.movingPiece;
		board[move.source] = nullptr;
	}
	colorToMove = Color::invert(colorToMove);
	attackMap = { 0 };

#ifdef _DEBUG
	moveStringHistory.push_back(move.toString());
#endif
	hashHistory.push_back(getHash());
	zobristHasher.updateHash(move);
}
void Board::unmakeMove(Move& move) {
	//en passant
	enpassantSquare = move.oldEnpassantSquare;
	enpassantPiece = move.oldEnpassantPiece;

	//castling rights
	castlingRights = move.oldCastlingRights;

	//update castling
	if (move.castlingMove == Move::Kingside)
	{
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square -= 2;
		move.capturedPiece->square += 2;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
	}
	else if (move.castlingMove == Move::Queenside)
	{
		board[move.movingPiece->square] = nullptr;
		board[move.capturedPiece->square] = nullptr;
		move.movingPiece->square += 2;
		move.capturedPiece->square -= 3;
		board[move.movingPiece->square] = move.movingPiece;
		board[move.capturedPiece->square] = move.capturedPiece;
	}
	else {
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
	}

	colorToMove = Color::invert(colorToMove);
	attackMap = { 0 };

#ifdef _DEBUG
	moveStringHistory.pop_back();
#endif
	hashHistory.pop_back();
	zobristHasher.updateHash(move);
}

void Board::print(std::ostream& out) {
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

void Board::cleanupDeadPieces()
{
	for (int c = 0; c < 2; c++) {
		std::vector<Piece*>* pieces = pieceListHolder[c];
		for (auto it = pieces->begin(); it != pieces->end(); ++it) {
			if (!(*it)->alive) {
				pieces->erase(it--);
			}
		}
	}
}

// getters
int Board::getColorToMove() {
	return colorToMove;
}
void Board::setColorToMove(int c) {
	colorToMove = c;
}
Piece* Board::getPiece(int pos) {
	return board[pos];
}
Piece* Board::getKing(int color) {
	return kings[color];
}
int Board::getEnpassantSquare() {
	return enpassantSquare;
}
Piece* Board::getEnpassantPiece() {
	return enpassantPiece;
}
CastlingRights Board::getCastlingRights() {
	return castlingRights;
}
std::vector<Piece*>* Board::getPieceList(int color) {
	return pieceListHolder[color];
}
int Board::getPieceCount(int color, Piece::PieceType type) {
	return pieceCount[color][type];
}
int Board::getTotalPieceCount(Piece::PieceType type) {
	return pieceCount[Color::WHITE][type] + pieceCount[Color::BLACK][type];
}
int Board::getNumberOfMoves() {
	return hashHistory.size();
}
bool Board::isEmptySquare(int square) {
	return board[square] == nullptr;
}
bool Board::isSquareOnBoard(int square) {
	return (square & 0x88) == 0;
}
int Board::convert88To64Square(int square0x88) {
	return (square0x88 + (square0x88 & 7)) >> 1;
}
int Board::convert64To88Square(int square64) {
	return square64 + (square64 & ~7);
}

std::string Board::getMoveStringAlgebraic(Move& move, bool requireUpperCasePromotionType) {
	if (move.castlingMove == Move::Kingside) {
		return "O-O";
	}
	else if (move.castlingMove == Move::Queenside) {
		return "O-O-O";
	}

	std::string result;
	Move moves[128];
	int n = generateMoves(move.color, moves);

	if (move.movingPiece->type == Piece::Pawn && move.capturedPiece != nullptr) {
		result += getFileBySquare(move.source);
	}
	else if (move.movingPiece->type != Piece::Pawn) {
		result += getCharOfPiece(move.movingPiece->type);
		bool needRank = false;
		bool needFile = false;
		for (int i = 0; i < n; i++) {
			Move& m = moves[i];
			if (m.movingPiece->type != move.movingPiece->type || m.destination != move.destination || m.equals(move)) {
				continue;
			}

			if (getFileBySquare(m.source) == getFileBySquare(move.source)) {
				needRank = true;
			}
			if (getRankBySquare(m.source) == getRankBySquare(move.source)) {
				needFile = true;
			}
			if (!needRank || !needFile)
				needFile = true;
		}
		if (needFile) {
			result += getFileBySquare(move.source);
		}
		if (needRank) {
			result += getRankBySquare(move.source);
		}
	}

	if (move.capturedPiece != nullptr) {
		result += 'x';
	}
	result += getFileBySquare(move.destination);
	result += getRankBySquare(move.destination);

	if (move.promotionType != Piece::None) {
		if (requireUpperCasePromotionType) {
			result += toupper(getCharOfPiece(move.promotionType));
		}
		else {
			result += tolower(getCharOfPiece(move.promotionType));
		}
	}

	makeMove(move);
	if (isMate(colorToMove)) {
		result += '#';
	}
	else if (inCheck(colorToMove)) {
		result += '+';
	}
	unmakeMove(move);

	return result;
}

char Board::getFileBySquare(int square) {
	return (square & 7) + 'a';
}
char Board::getRankBySquare(int square) {
	return (square >> 4) + '1';
}

int Board::getSquareFromString(std::string str) {
	return str[0] - 'a' + 16 * (str[1] - '1');
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

Piece::PieceType Board::getPieceTypeFromChar(char type)
{
	switch (type) {
	case 'Q':
	case 'q':
		return Piece::Queen;
	case 'R':
	case 'r':
		return Piece::Rook;
	case 'N':
	case 'n':
		return Piece::Knight;
	case 'B':
	case 'b':
		return Piece::Bishop;
	case 'K':
	case 'k':
		return Piece::King;
	case 'P':
	case 'p':
		return Piece::Pawn;
	}
	return Piece::None;
}

u64 Board::getHash() {
	return zobristHasher.getHash();
}