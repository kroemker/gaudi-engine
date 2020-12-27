#include "LuaEvaluator.h"
#include "../luafuncs.h"
#include <iostream>

LuaEvaluator::LuaEvaluator(Board* board, lua_State* luaState, u64 tableSize) : evalTable(tableSize) {
	this->board = board;
	this->luaState = luaState;
	setup_lua(board);
	lua_pushcfunction(luaState, l_getPieceCount);
	lua_setglobal(luaState, "getPieceCount");

	lua_pushcfunction(luaState, l_getPieceInfo);
	lua_setglobal(luaState, "getPieceInfo");

	lua_pushcfunction(luaState, l_getMoveCount);
	lua_setglobal(luaState, "getMoveCount");

	lua_pushcfunction(luaState, l_getMoveInfo);
	lua_setglobal(luaState, "getMoveInfo");

	lua_pushcfunction(luaState, l_getFileRank);
	lua_setglobal(luaState, "getFileRank");

	lua_pushcfunction(luaState, l_getBoard);
	lua_setglobal(luaState, "getBoard");
}

int LuaEvaluator::evaluate() {
	EvaluationEntry* evalEntry = evalTable.find(board->getHash());
	if (evalEntry->hash == board->getHash()) {
		return evalEntry->score;
	}

	int s = 0;

	lua_getglobal(luaState, "evaluate");

	if (lua_pcall(luaState, 0, 1, 0) == LUA_OK) {
		s = lua_tointeger(luaState, -1);
		lua_pop(luaState, 1);
	}
	else {
		std::cerr << "Error running evaluate(): " << lua_tostring(luaState, -1) << std::endl;
	}

	s = board->getColorToMove() == Color::WHITE ? s : -s;

	evalTable.store(EvaluationEntry(board->getHash(), s));

	return s;
}