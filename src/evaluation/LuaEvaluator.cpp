#include "LuaEvaluator.h"
#include <iostream>

LuaEvaluator::LuaEvaluator(Board* board, lua_State* luaState, u64 tableSize) : evalTable(tableSize) {
	this->board = board;
	this->luaState = luaState;
}

int LuaEvaluator::evaluate() {
	EvaluationEntry* evalEntry = evalTable.find(board->getHash());
	if (evalEntry->hash == board->getHash()) {
		return evalEntry->score;
	}

	int s = 0;

	lua_getglobal(luaState, "evaluate");

	if (lua_pcall(luaState, 0, 1, 0) == LUA_OK) {
		s = lua_tonumber(luaState, -1);
		lua_pop(luaState, 1);
	}
	else {
		std::cerr << "Error running evaluate(): " << lua_tostring(luaState, -1) << std::endl;
	}

	evalTable.store(EvaluationEntry(board->getHash(), s));

	return s;
}