#include "Searcher.h"
#include "evaluation/DefaultEvaluator.h"

#include <iostream>
#include <vector>
#include <climits>
#include <algorithm>

Searcher::Searcher(Board* board, Evaluator* evaluator, HashTable<TranspositionEntry>* transTable, Log* log) : moveComparator(board, transTable) {
	this->board = board;
	this->evaluator = evaluator;
	this->transTable = transTable;
	this->log = log;
}

void Searcher::search(int depth, int timeLimitMs) {
	int d = 2;
	int score = 0;
	timeUp = false;

	timeLimit = timeLimitMs;
	nodes = 0;
	quiesceNodes = 0;
	tableHits = 0;
	quiesceTableHits = 0; 
	evaluations = 0;

	beginSearch = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point lastSearch = beginSearch;
	int prevScore = 0;
	while (d <= depth) {
		prevScore = score;
		score = pvSearchRoot(d);

		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		int timeLeft = timeLimit - std::chrono::duration_cast<std::chrono::milliseconds>(now - beginSearch).count();
		int lastIterationTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSearch).count();
		int nextIterationEstimate = lastIterationTime * 3; //d;
		if (timeUp) {
			if (d == 2) {
				log->writeMessage("Exiting in first iteration. Choosing random move...");
			}
			else {
				d--;
				score = prevScore;
			}
			break;
		}
		if (nextIterationEstimate > timeLeft) {
			log->getStream() << "Exiting search: depth = " << d <<
				", last iteration time = " << (double)lastIterationTime / 1000.0 <<
				"s, estimated next iteration time = " << (double)nextIterationEstimate / 1000.0 <<
				"s, time left = " << (double)timeLeft / 1000.0 << "s" << std::endl;
			break;
		}

		d++;
	}
	int allNodes = quiesceNodes + nodes;
	int allTableHits = tableHits + quiesceTableHits;
	log->getStream() << "Searching depth: " << d << std::endl;
	log->getStream() << "Search Nodes : " << nodes << "(" << (double)nodes/(double)allNodes * 100.0 << "%)" <<
		", Quiescence Nodes: " << quiesceNodes << "(" << (double)quiesceNodes / (double)allNodes * 100.0 << "%)" <<
		", Nodes: " << allNodes << std::endl;
	log->getStream() << "Search Table Hits : " << tableHits << "(" << (double)tableHits / (double)allTableHits * 100.0 << "%)" <<
		", Quiescent Table Hits: " << quiesceTableHits << "(" << (double)quiesceTableHits / (double)allTableHits * 100.0 << "%)" <<
		", Table Hits: " << allTableHits << std::endl;
	log->getStream() << "Evaluations: " << evaluations << std::endl;
	log->getStream() << "Best move: " << board->getMoveStringAlgebraic(bestMove) << " , score: " << (float)score / 100.0f << std::endl;
	log->writePV();
	log->writeBoard();
}

int Searcher::pvSearchRoot(int depth) {
	int alpha = -MAX_SCORE;
	int beta = MAX_SCORE;

	nodes++;

	Move moves[128];
	int n = board->generateMoves(board->getColorToMove(), moves);
	std::sort(moves, moves + n, moveComparator);

	int score;
	int bestMoveIndex = -1;
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];
		board->makeMove(m);

		if (board->inCheck(m.color)) {
			board->unmakeMove(m);
			continue;
		}

		if (i == 0 || -pvSearch(-alpha - 1, -alpha, depth - 1, false) > alpha) {
			score = -pvSearch(-beta, -alpha, depth - 1, true);
			if (score > alpha && !timeUp) {
				bestMoveIndex = i;
				alpha = score;
			}
		}
		board->unmakeMove(m);
		if (timeUp) {
			// if we didn't find a move in the first iteration, take a move of which we know it is legal
			if (depth == 2) {
				bestMove = moves[bestMoveIndex < 0 ? i : bestMoveIndex];
			}
			return score;
		}
	}
	
	bestMove = moves[bestMoveIndex];
	transTable->store(TranspositionEntry(board->getHash(), depth, alpha, TranspositionEntry::HASH_EXACT, bestMove));
	return alpha;
}

Move Searcher::getBestMove() {
	return bestMove;
}

int Searcher::pvSearch(int alpha, int beta, int depth, bool pvNode) {
	if (timeUp) {
		return 0;
	}
	else if ((nodes & 4095) == 0) {
		checkTimeUp();
	}

	if (depth == 0) {
		return quiesce(alpha, beta);
	}

	if (!board->sufficientMaterial() || board->isRepetition()) {
		return 0;
	}

	TranspositionEntry* entry = dynamic_cast<TranspositionEntry*>(transTable->find(board->getHash()));
	if (entry->hash == board->getHash() && entry->depth >= depth) {
		if (entry->flag == TranspositionEntry::HASH_EXACT) {
			tableHits++;
			return entry->score;
		}
		else if (!pvNode && entry->flag == TranspositionEntry::HASH_ALPHA && entry->score <= alpha) {
			tableHits++;
			return alpha;
		}
		else if (!pvNode && entry->flag == TranspositionEntry::HASH_BETA && entry->score >= beta) {
			tableHits++;
			return beta;
		}
	}

	nodes++;

	Move moves[128];
	int n = board->generateMoves(board->getColorToMove(), moves);

	std::sort(moves, moves + n, moveComparator);

	int score;
	int bestMoveIndex = -1;
	bool hasLegalMove = false;
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];

		board->makeMove(m);

		if (board->inCheck(m.color)) {
			board->unmakeMove(m);
			continue;
		}

		hasLegalMove = true;

		if (i == 0 || -pvSearch(-alpha - 1, -alpha, depth - 1, false) > alpha) {
			score = -pvSearch(-beta, -alpha, depth - 1, true);
			if (score > alpha && !timeUp) {
				if (score >= beta) {
					board->unmakeMove(m);
					transTable->store(TranspositionEntry(board->getHash(), depth, beta, TranspositionEntry::HASH_BETA, m));
					return beta;
				}
				alpha = score;
				bestMoveIndex = i;
			}
		}
		board->unmakeMove(m);

		if (timeUp) {
			return 0;
		}
	}

	// check mate and stalemate
	if (!hasLegalMove) {
		if (board->inCheck(board->getColorToMove())) {
			return -MATE_SCORE * depth; // prefer near mates
		}
		else {
			return 0;
		}
	}

	if (bestMoveIndex < 0) {
		transTable->store(TranspositionEntry(board->getHash(), depth, alpha, TranspositionEntry::HASH_ALPHA, Move()));
	}
	else {
		transTable->store(TranspositionEntry(board->getHash(), depth, alpha, TranspositionEntry::HASH_EXACT, moves[bestMoveIndex]));
	}

	return alpha;
}

int Searcher::quiesce(int alpha, int beta) {
	if (timeUp) {
		return 0;
	}
	else if ((quiesceNodes & 8191) == 0) {
		checkTimeUp();
	}

	evaluations++;
	int standPattern = evaluator->evaluate();
	if (standPattern >= beta) {
		return beta;
	}

	// check if capturing a queen could raise alpha 
	if (standPattern + DefaultEvaluator::PIECE_WORTH[Piece::Queen] < alpha) {
		return alpha;
	}

	if (alpha < standPattern) {
		alpha = standPattern;
	}

	quiesceNodes++;

	Move captures[128];
	int n = board->generateCaptures(board->getColorToMove(), captures);
	std::sort(captures, captures + n, moveComparator);

	int score;
	for (int i = 0; i < n; i++) {
		Move& m = captures[i];
		if (m.capturedPiece->type == Piece::King) {
			return MATE_SCORE;
		}
		// delta pruning
		if (standPattern + DefaultEvaluator::PIECE_WORTH[m.capturedPiece->type] + 200 < alpha) {
			continue;
		}

		board->makeMove(m);
		if (board->inCheck(m.color)) {
			board->unmakeMove(m);
			continue;
		}
		score = -quiesce(-beta, -alpha);
		board->unmakeMove(m);

		if (timeUp) {
			return 0;
		}
		
		if (score > alpha) {
			alpha = score; 
			if (score >= beta) {
				return beta;
			}
		}
	}
	return alpha;
}

void Searcher::checkTimeUp() {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	timeUp = std::chrono::duration_cast<std::chrono::milliseconds>(now - beginSearch).count() > timeLimit;
}

void Searcher::setEvaluator(Evaluator* evaluator) {
	this->evaluator = evaluator;
}

void Searcher::test(std::string fen, int depth) {
	board->loadFEN(fen);

	search(depth, 5000);
	log->writePV();

	Move moves[128];
	int n = board->generateMoves(board->getColorToMove(), moves);
	std::sort(moves, moves + n, moveComparator);

	log->writeBoard();
	log->getStream() << "Ordered moves: ";
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];
		log->getStream() << board->getMoveStringAlgebraic(m) << " ";
	}
	log->getStream() << std::endl;
}

void Searcher::assertBoardHash(u64 should) {
	if (should != board->getHash())
	{
		std::cout << "PRE/AFTER HASHES DO NOT MATCH: " << should << " vs " << board->getHash() << std::endl;
		board->print(std::cout);
	}
}