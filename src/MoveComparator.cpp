#include "MoveComparator.h"
#include "evaluation/DefaultEvaluator.h"

MoveComparator::MoveComparator(Board* board, HashTable<TranspositionEntry>* transTable) {
	this->board = board;
	this->transTable = transTable;
}

bool MoveComparator::operator()(Move& m1, Move& m2) {
	TranspositionEntry* entry = transTable->find(board->getHash());

	if (entry->hash == board->getHash() && entry->bestMove.source != entry->bestMove.destination) {
		if (entry->bestMove.equals(m1)) {
			return true;
		}
		else if (entry->bestMove.equals(m2)) {
			return false;
		}
	}

	if (m1.capturedPiece != nullptr && m2.capturedPiece == nullptr) {
		return true;
	}
	if (m1.capturedPiece == nullptr && m2.capturedPiece != nullptr) {
		return false;
	}
	else if (m1.capturedPiece != nullptr && m2.capturedPiece != nullptr) {
		int d1 = DefaultEvaluator::PIECE_WORTH[m1.movingPiece->type] - DefaultEvaluator::PIECE_WORTH[m1.capturedPiece->type];
		int d2 = DefaultEvaluator::PIECE_WORTH[m2.movingPiece->type] - DefaultEvaluator::PIECE_WORTH[m2.capturedPiece->type];
		if (d1 != d2) {
			return d1 < d2;
		}
		else {
			return m1.capturedPiece->type > m2.capturedPiece->type;
		}
	}

	return m1.source > m2.source;
}