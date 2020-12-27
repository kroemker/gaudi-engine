#include "DefaultEvaluator.h"
#include "../Piece.h"

const int DefaultEvaluator::PIECE_WORTH[] = { 0, 100, 300, 315, 500, 1000 };

static const int sKingSafetyContribution[] = { 0, 4, 1, 2, 3, 1 };

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

static const int queenVectorMoves[] = { -1, 1, 16, -16, -15, -17, 15, 17 };

DefaultEvaluator::DefaultEvaluator(Board* board, u64 tableSize) : evalTable(tableSize) {
	this->board = board;
}

int DefaultEvaluator::evaluate() {
	EvaluationEntry* evalEntry = evalTable.find(board->getHash());
	if (evalEntry->hash == board->getHash()) {
		return evalEntry->score;
	}

	int color = board->getColorToMove();
	int oppColor = Color::invert(board->getColorToMove());
	int s = 0;
	int materialOnBoard = 0;

	// material
	for (int i = 0; i < 6; i++) {
		s += PIECE_WORTH[i] * (board->getPieceCount(color, (Piece::PieceType)i) - board->getPieceCount(oppColor, (Piece::PieceType)i));
		materialOnBoard += PIECE_WORTH[i] * board->getPieceCount(color, (Piece::PieceType)i) + PIECE_WORTH[i] * board->getPieceCount(oppColor, (Piece::PieceType)i);
	}

	// pawn/knight position
	u8 pawnsOnFiles[2][8] = { 0 };
	for (int c = 0; c < 2; c++) {
		int sign = color == c ? 1 : -1;
		std::vector<Piece*>* pieces = board->getPieceList(c);
		for (int i = 0; i < pieces->size(); i++) {
			Piece* p = (*pieces)[i];
			if (!p->alive)
				continue;
			switch (p->type) {
			case Piece::Pawn:
				s += sign * sPawnTable[c][p->square];
				pawnsOnFiles[c][p->square & 7]++;
				break;
			case Piece::Knight:
				s += sign * sKnightPositionalValueTable[p->square];
				break;
			}
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
		if (m.capturedPiece != nullptr && m.capturedPiece->type > m.movingPiece->type) {
			s += 3;
		}
		// dont count king/queen mobility
		if (m.movingPiece->type == Piece::Queen || m.movingPiece->type == Piece::King) {
			s--;
		}
	}

	int m = board->generateMoves(oppColor, moves);
	for (int i = 0; i < m; i++) {
		Move& m = moves[i];
		if (m.capturedPiece != nullptr && m.capturedPiece->type > m.movingPiece->type) {
			s -= 3;
		}
		// dont count king/queen mobility
		if (m.movingPiece->type == Piece::Queen || m.movingPiece->type == Piece::King) {
			s++;
		}
	}
	// move count
	s += (n - m);

	// king safety
	for (int c = 0; c < 2; c++) {
		int sign = color == c ? 1 : -1;
		Piece* king = board->getKing(c);
		for (int i = 0; i < 8; i++) {
			int sq = king->square + queenVectorMoves[i];
			while (board->isSquareOnBoard(sq)) {
				Piece* hitPiece;
				if ((hitPiece = board->getPiece(sq)) != nullptr && hitPiece->color == c) {
					break;
				}
				s -= sign;
				sq += queenVectorMoves[i];
			}
		}
	}

	evalTable.store(EvaluationEntry(board->getHash(), s));

	return s;
}