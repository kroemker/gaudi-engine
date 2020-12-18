#include "Engine.h"
#include "Board.h"
#include "types.h"
#include <cctype>
#include <string>
#include <iostream>
#include <cstdlib>

const std::string Engine::engineName = "gaudi-engine";
static u64 transTableDefaultSize = 1006003;

Engine::Engine() : board(), searcher(&board, &evaluator, &transTable, &log), evaluator(&board), transTable(transTableDefaultSize), log(&board, &transTable) {
	searchDepth = 20;
	clockHandler.setMoveTime(60000);
}

Move Engine::move() {
	board.cleanupDeadPieces();
	int time = clockHandler.getSearchTime(board.getColorToMove());
	log.getStream() << "Start search with depth " << searchDepth << " and time " << (double)time / 1000.0 << "s" << std::endl;
	searcher.search(searchDepth, time);
	Move move = searcher.getBestMove();
	board.makeMove(move);
	return move;
}

void Engine::startNewGame() {
	board.loadStartPosition();
	log.writeMessage("Preparing for new game...");
	log.writeBoard();
}

void Engine::setBoard(std::string fen) {
	board.loadFEN(fen);
}

void Engine::doMove(std::string move) {
	const int rookSquares[2][2] = {
		{ 0, 7 },
		{ 112, 119}
	};

	Move m;

	if (move.length() < 4 || !isalpha(move[0]) || !isalpha(move[2]) || !isdigit(move[1]) || !isdigit(move[3])) {
		return;
	}

	int k = move[0] - 'a';
	int q = move[1] - '1';
	int src = k + q * 16;

	k = move[2] - 'a';
	q = move[3] - '1';
	int dest = k + q * 16;

	Piece::PieceType promotionType = Piece::None;
	if (move.length() == 5) {
		promotionType = Board::getPieceTypeFromChar(move[4]);
	}

	int color = board.getColorToMove();
	Piece* king = board.getKing(color);
	Piece* queenRook = board.getPiece(rookSquares[color][0]);
	Piece* kingRook = board.getPiece(rookSquares[color][1]);
	int enpSq = board.getEnpassantSquare();
	Piece* enpP = board.getEnpassantPiece();
	CastlingRights cr = board.getCastlingRights();
	Piece* srcPiece = board.getPiece(src);
	Piece* destPiece = board.getPiece(dest);

	if ((move == "e1g1" && king->square == board.getSquareFromString("e1")) || (move == "e8g8" && king->square == board.getSquareFromString("e8"))) {
		m = Move(color, Move::Kingside, king, kingRook, dest, dest - 1, enpSq, enpP, cr);
	}
	else if ((move == "e1c1" && king->square == board.getSquareFromString("e1")) || (move == "e8c8" && king->square == board.getSquareFromString("e8"))) {
		m = Move(color, Move::Queenside, king, queenRook, dest, dest + 1, enpSq, enpP, cr);
	}
	else if (promotionType != Piece::None) {
		m = Move(color, src, dest, srcPiece, destPiece, enpSq, enpP, cr, promotionType);
	}
	else if (board.getPiece(src)->type == Piece::Pawn && dest == enpSq) {
		m = Move(color, src, dest, srcPiece, enpP, enpSq, enpP, cr);
	}
	else {
		m = Move(color, src, dest, srcPiece, destPiece, enpSq, enpP, cr);
	}

	board.makeMove(m);
}

Log* Engine::getLog() {
	return &log;
}

void Engine::setClockTime(int color, int timeMs) {
	clockHandler.setClockTime(color, timeMs);
}

void Engine::setClockIncrement(int color, int timeMs) {
	clockHandler.setClockIncrement(color, timeMs);
}

void Engine::setMoveTime(int timeMs) {
	clockHandler.setMoveTime(timeMs);
}

void Engine::setSearchDepth(int depth) {
	searchDepth = depth;
}

void Engine::runTests() {
	Move moves[128];
	int n;
	bool failed = false;

	std::cout << "Testing hashing...";
	log.writeMessage("Testing hashing...");
	board.loadFEN("r1b1kb1r/p3p2p/nq6/1p3pp1/8/5Q1N/PPPP1nPP/RNB1K2R w KQkq - 0 1");
	log.writeBoard();
	n = board.generateMoves(Color::WHITE, moves);
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];
		u64 h = board.getHash();
		log.getStream() << "Testing move: " << board.getMoveStringAlgebraic(m) << " ...";
		board.makeMove(m);
		if (h == board.getHash()) {
			log.getStream() << "HASH DID NOT CHANGE...";
		}
		board.unmakeMove(m);
		if (h != board.getHash()) {
			log.getStream() << "HASH FAIL...";
			failed = true;
		}
		log.getStream() << std::endl;
	}
	if (failed) {
		std::cout << "FAILED" << std::endl;
	}
	else {
		std::cout << "SUCCESS" << std::endl;
	}
	log.getStream() << std::endl;


	std::cout << "Testing move generation...";
	log.writeMessage("Testing move generation...");
	board.loadFEN("r1bqk1nr/pppp1ppp/2P5/8/8/b7/PPP1PPPP/RNBQKBNR w KQkq - 0 1");
	log.writeBoard();
	n = board.generateMoves(Color::WHITE, moves);
	if (n != 32) {
		std::cout << "FAILED" << std::endl;
	}
	else {
		std::cout << "SUCCESS" << std::endl;
	}
	log.getStream() << "Moves: ";
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];
		log.getStream() << board.getMoveStringAlgebraic(m) << " ";
	}
	log.getStream() << std::endl;
	log.getStream() << n << " moves, should be " << 32 << std::endl;

	std::cout << "Testing capture generation...";
	log.writeMessage("Testing capture generation...");
	board.loadFEN("r1b1kb1r/1p1n1p1p/pq1p1np1/2p1p1B1/1PBPP3/2N2P1N/P1P3PP/R2QK2R b KQkq - 2 10");
	log.writeBoard();
	n = board.generateCaptures(Color::BLACK, moves);
	if (n != 5) {
		std::cout << "FAILED" << std::endl;
	}
	else {
		std::cout << "SUCCESS" << std::endl;
	}
	log.getStream() << "Captures: ";
	for (int i = 0; i < n; i++) {
		Move& m = moves[i];
		log.getStream() << board.getMoveStringAlgebraic(m) << " ";
	}
	log.getStream() << std::endl;
	log.getStream() << n << " captures, should be " << 5 << std::endl;

	std::cout << "Testing move operation...";
	log.writeMessage("");
	log.writeMessage("Testing move operation...");
	board.loadStartPosition();
	board.generateMoves(rand() % 2, moves);
	u64 hash = board.getHash();
	board.makeMove(moves[0]);
	board.unmakeMove(moves[0]);
	if (hash != board.getHash()) {
		std::cout << "FAILED" << std::endl;
		log.writeMessage("FAILED");
	}
	else {
		std::cout << "SUCCESS" << std::endl;
		log.writeMessage("SUCCESS");
	}

	std::cout << "Testing move comparison...";
	log.writeMessage("");
	log.writeMessage("Testing move comparison...");
	searcher.test("r1b1kb1r/p3p2p/nq6/1p3pp1/8/5Q1N/PPPP1nPP/RNB1K2R w KQkq - 0 1", 5);
	if (searcher.getBestMove().source != board.getSquareFromString("f3") || searcher.getBestMove().destination != board.getSquareFromString("a8")) {
		std::cout << "FAILED" << std::endl;
	}
	else {
		std::cout << "SUCCESS" << std::endl;
	}

	std::cout << "Testing evaluation...";
	log.writeMessage("");
	log.writeMessage("Testing move evaluation...");
	board.loadFEN("k7/p1p5/1p4p1/5p2/P2P4/7P/P6P/K7 w - - 0 1");
	log.writeBoard();
	int eval = evaluator.evaluate();
	log.getStream() << "Evaluation: " << (float)eval / 100.0f;
	if (eval > 0) {
		std::cout << "FAILED" << std::endl;
	}
	else {
		std::cout << "SUCCESS" << std::endl;
	}

}