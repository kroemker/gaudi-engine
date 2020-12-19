#include "Evaluator.h"
#include "Piece.h"
#include <iostream>

const int Evaluator::PIECE_WORTH[] = { 0, 100, 280, 310, 600, 900 };

static const int sWhitePawnPositionalValueTable[] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0,
  4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0,
  6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
static const int sBlackPawnPositionalValueTable[] =
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0,
  4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const int sKnightPositionalValueTable[] =
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 4, 4, 4, 4, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 4, 5, 5, 4, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 4, 5, 5, 4, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 3, 4, 4, 4, 4, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 2, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

static const int* sPawnTable[] = { sWhitePawnPositionalValueTable, sBlackPawnPositionalValueTable };

Evaluator::Evaluator(Board* board, u64 tableSize) : evalTable(tableSize) {
	this->board = board;
}

int Evaluator::evaluate() {
	EvaluationEntry* evalEntry = evalTable.find(board->getHash());
	if (evalEntry->hash == board->getHash()) {
		return evalEntry->score;
	}

	int color = board->getColorToMove();
	int oppColor = Color::invert(board->getColorToMove());
	int s = 0;

	// material
	for (int i = 0; i < 6; i++) {
		s += PIECE_WORTH[i] * (board->getPieceCount(color, (Piece::PieceType)i) - board->getPieceCount(oppColor, (Piece::PieceType)i));
	}

	// pawn/knight position
	std::vector<Piece*>* myPieces = board->getPieceList(color);
	u8 pawnsOnFiles[2][8] = { 0 };
	for (int i = 0; i < myPieces->size(); i++) {
		Piece* p = (*myPieces)[i];
		if (!p->alive)
			continue;
		switch (p->type) {
		case Piece::Pawn:
			s += sPawnTable[color][p->square];
			pawnsOnFiles[color][p->square & 7]++;
			break;
		case Piece::Knight:
			s += sKnightPositionalValueTable[p->square];
			break;
		}
	}

	std::vector<Piece*>* oppPieces = board->getPieceList(oppColor);
	for (int i = 0; i < oppPieces->size(); i++) {
		Piece* p = (*oppPieces)[i];
		if (!p->alive)
			continue;
		switch (p->type) {
		case Piece::Pawn:
			s -= sPawnTable[oppColor][p->square];
			pawnsOnFiles[oppColor][p->square & 7]++;
			break;
		case Piece::Knight:
			s -= sKnightPositionalValueTable[p->square];
			break;
		}
	}

	// double, isolated, passed pawns
	int pawnEval = 0;
	for (int i = 0; i < 8; i++) {
		for (int c = 0; c < 2; c++) {
			int sign = color == c ? 1 : -1;
			// double pawns
			if (pawnsOnFiles[c][i] > 1) {
				pawnEval -= sign;
			}
			// isolated pawns
			if (pawnsOnFiles[c][i] > 0 && (i == 0 || pawnsOnFiles[c][i - 1] == 0) && (i == 7 || pawnsOnFiles[c][i + 1] == 0)) {
				pawnEval -= sign;
			}
			// passed
			if (pawnsOnFiles[c][i] > 0 && pawnsOnFiles[c ^ 1][i] == 0 && (i == 0 || pawnsOnFiles[c ^ 1][i - 1] == 0) && (i == 7 || pawnsOnFiles[c ^ 1][i + 1] == 0)) {
				pawnEval += 2 * sign;
			}
		}
	}
	s += 3 * pawnEval;

	// tempo
	Move moves[128];
	int n = board->generateMoves(color, moves);
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];
		if (m.capture && m.capturedPiece->type > m.movingPiece->type) {
			s += 3;
		}
		// dont count queen mobility
		if (m.movingPiece->type == Piece::Queen) {
			s--;
		}
	}

	int m = board->generateMoves(oppColor, moves);
	for (int i = 0; i < m; i++) {
		Move& m = moves[i];
		if (m.capture && m.capturedPiece->type > m.movingPiece->type) {
			s -= 3;
		}
		// dont count queen mobility
		if (m.movingPiece->type == Piece::Queen) {
			s++;
		}
	}
	// move count
	s += (n - m);

	evalTable.store(EvaluationEntry(board->getHash(), s));

	return s;
}