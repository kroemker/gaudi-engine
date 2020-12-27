#include "luafuncs.h"

static Board* board;
static Move moves[2][128];

void setup_lua(Board* b) {
	board = b;
}

// getPieceCount(color) -> returns number of pieces of 'color'
int l_getPieceCount(lua_State* L) {
	int c = lua_tointeger(L, 1);
	lua_pushinteger(L, board->getPieceList(c)->size());
	return 1;
}

// getPieceInfo(color, index) -> returns 'alive', 'type', 'square' for piece 'index' of 'color'
int l_getPieceInfo(lua_State* L) {
	int c = lua_tointeger(L, 1);
	int i = lua_tointeger(L, 2);
	std::vector<Piece*>* pieceList = board->getPieceList(c);
	if (i > pieceList->size() || i < 0) {
		lua_pushnil(L);
		lua_pushstring(L, "Invalid index for piece list!");
		return 2;
	}
	Piece* p = (*pieceList)[i];

	lua_pushboolean(L, p->alive);
	lua_pushinteger(L, p->type);
	lua_pushinteger(L, Board::convert88To64Square(p->square));

	return 3;
}

// getMoveCount(color) -> returns number of pseudo-legal moves of 'color'
int l_getMoveCount(lua_State* L) {
	int c = lua_tointeger(L, 1);
	int n = board->generateMoves(c, &moves[c][0]);
	lua_pushinteger(L, n);
	return 1;
}

// getMoveInfo(color, index) -> returns 'source_square', 'destination_square', 'moving_piece_type', 'captured_piece_type', 'castling_move' for move 'index' of 'color'
int l_getMoveInfo(lua_State* L) {
	int c = lua_tointeger(L, 1);
	int i = lua_tointeger(L, 2);
	if (i >= 128 || i < 0) {
		lua_pushnil(L);
		lua_pushstring(L, "Invalid index for move list!");
		return 2;
	}
	Move& m = moves[c][i];
	lua_pushinteger(L, Board::convert88To64Square(m.source));
	lua_pushinteger(L, Board::convert88To64Square(m.destination));
	lua_pushinteger(L, m.movingPiece->type);
	lua_pushinteger(L, m.capturedPiece != nullptr ? m.capturedPiece->type : -1);
	lua_pushinteger(L, m.castlingMove != Move::None ? m.castlingMove : -1);
	return 5;
}

// getFileRank(square) -> returns 'file', 'rank', of the 8x8 board square
int l_getFileRank(lua_State* L) {
	int sq = lua_tointeger(L, 1);
	lua_pushinteger(L, sq & 7);
	lua_pushinteger(L, sq >> 3);
	return 2;
}

// getBoard() -> returns array[0..63] with values -1(no piece) to 6
int l_getBoard(lua_State* L) {
	lua_newtable(L);
	int top = lua_gettop(L);
	for (int i = 0; i < 64; i++) {
		Piece* p = board->getPiece(Board::convert64To88Square(i));
		lua_pushinteger(L, i);
		lua_pushinteger(L, p != nullptr ? p->type : -1);
		lua_settable(L, top);
	}
	return 1;
}