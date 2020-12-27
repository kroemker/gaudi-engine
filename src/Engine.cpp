#include "Engine.h"
#include "Board.h"
#include "types.h"
#include "evaluation/DefaultEvaluator.h"
#include "evaluation/LuaEvaluator.h"

#include <string>
#include <iostream>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <ctime>

Engine::Engine(Configuration* configuration) :
	searcher(&board, evaluator, &transTable, &log), 
	transTable(configuration->transpositionTableSize), 
	log(&board, &transTable, configuration->path + "logs/"), 
	clockHandler(&board) {

	engineName = configuration->engineName;
	searchDepth = 20;
	clockHandler.setMoveTime(10000);
	if (!configuration->luaFilename.empty()) {
		luaState = luaL_newstate();
		luaL_openlibs(luaState);
		if (luaL_dofile(luaState, configuration->luaFilename.c_str()) != LUA_OK) {
			std::cerr << "Can't load " << configuration->luaFilename << ": " << lua_tostring(luaState, -1) << std::endl;
			luaState = nullptr;
		}
	}
	else {
		luaState = nullptr;
	}

	if (luaState == nullptr) {
		evaluator = new DefaultEvaluator(&board, configuration->evaluationTableSize);
		std::cerr << "Using default evaluation..." << std::endl;
		log.writeMessage("Using default evaluation...");
	}
	else {
		evaluator = new LuaEvaluator(&board, luaState, configuration->evaluationTableSize);
		std::cerr << "Using custom lua evaluation..." << std::endl;
		log.writeMessage("Using custom lua evaluation...");
	}
	searcher.setEvaluator(evaluator);
}

Engine::~Engine() {
	if (evaluator != nullptr) {
		delete evaluator;
	}
}

Move Engine::move() {
	board.cleanupDeadPieces();
	int time = clockHandler.getSearchTime(board.getColorToMove());
	log.writeDelimiter();
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

void Engine::playSelf(int clockMode, int time, int increment) {
	std::time_t now = std::time(nullptr);
	tm* localTime = localtime(&now);

	pgn.setWhiteName(engineName);
	pgn.setBlackName(engineName);
	pgn.setEvent("Self-Play");
	pgn.setDate(1900 + localTime->tm_year, 1 + localTime->tm_mon, localTime->tm_mday);

	board.loadStartPosition();
	int wtime = time;
	int btime = time;
	setClockIncrement(Color::WHITE, increment);
	setClockIncrement(Color::BLACK, increment);
	bool gameOver = false;
	int moves = 1;
	while (!gameOver) {
		// update clock
		if (clockMode == ClockHandler::NORMAL) {
			if (board.getColorToMove() == Color::WHITE) {
				setClockTime(Color::WHITE, wtime);
			}
			else {
				setClockTime(Color::BLACK, btime);
			}
		}
		else if (clockMode == ClockHandler::MOVETIME) {
			setMoveTime(time);
		}

		// search move
		std::chrono::steady_clock::time_point beginSearch = std::chrono::steady_clock::now();
		Move m = move();
		board.unmakeMove(m);

		// update time
		if (clockMode == ClockHandler::NORMAL) {
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			if (m.color == Color::WHITE) {
				wtime -= std::chrono::duration_cast<std::chrono::milliseconds>(now - beginSearch).count();
				wtime += increment;
			}
			else {
				btime -= std::chrono::duration_cast<std::chrono::milliseconds>(now - beginSearch).count();
				btime += increment;
			}
		}
		
		// output move
		pgn.addMove(board.getMoveStringAlgebraic(m, true));
		if (board.getColorToMove() == Color::WHITE) {
			std::cout << moves << ". " << board.getMoveStringAlgebraic(m) << " ";
		}
		else {
			std::cout << board.getMoveStringAlgebraic(m);
			if (clockMode == ClockHandler::NORMAL) {
				std::cout << " \t" << (double)wtime / 1000.0 << "s / " << (double)btime / 1000.0 << "s";
			}
			std::cout << std::endl;
			moves++;
		}
		board.makeMove(m);

		// check game end
		if (wtime <= 0) {
			std::string result = "{ White out of time } 0-1";
			std::cout << result << std::endl;
			log.getStream() << result << std::endl;
			pgn.setResult("0-1");
			gameOver = true;
		}
		else if (btime <= 0) {
			std::string result = "{ Black out of time } 1-0";
			std::cout << result << std::endl;
			log.getStream() << result << std::endl;
			pgn.setResult("1-0");
			gameOver = true;
		}
		else if (board.isMate(board.getColorToMove())) {
			if (board.getColorToMove() == Color::BLACK) {
				std::string result = "{ White checkmates } 1-0";
				std::cout << result << std::endl;
				log.getStream() << result << std::endl;
				pgn.setResult("1-0");
			}
			else {
				std::string result = "{ Black checkmates } 0-1";
				std::cout << result << std::endl;
				log.getStream() << result << std::endl;
				pgn.setResult("0-1");
			}
			gameOver = true;
		}
		else if (board.isRepetition()) {
			std::string result = "{ 3-time repetition } 1/2-1/2";
			std::cout << result << std::endl;
			log.getStream() << result << std::endl;
			pgn.setResult("1/2-1/2");
			gameOver = true;
		}
		else if (board.isStalemate(board.getColorToMove())) {
			std::string result = "{ Stalemate } 1/2-1/2";
			std::cout << result << std::endl;
			log.getStream() << result << std::endl;
			pgn.setResult("1/2-1/2");
			gameOver = true;
		}
	}
	pgn.writePGN();
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
	int eval = evaluator->evaluate();
	log.getStream() << "Evaluation: " << (float)eval / 100.0f;
	if (eval > 0) {
		std::cout << "FAILED" << std::endl;
	}
	else {
		std::cout << "SUCCESS" << std::endl;
	}

}

void Engine::evaluatePosition(std::string fen) {
	if (!fen.empty()) {
		board.loadFEN(fen);
	}
	else {
		board.loadStartPosition();
	}

	std::cout << "Testing evaluation for position " << fen << std::endl;
	board.print(std::cout);
	int score = evaluator->evaluate();
	score = board.getColorToMove() == Color::WHITE ? score : -score;
	std::cout << "Evaluation score: " << (double)score / 100.0 << std::endl;
}