#pragma once
#include "Evaluator.h"
#include <lua.hpp>
#include "../types.h"
#include "../Board.h"
#include "../hashing/EvaluationEntry.h"
#include "../hashing/HashTable.h"

class LuaEvaluator : public Evaluator
{
public:
	LuaEvaluator(Board* board, lua_State* luaState, u64 tableSize = 1000001);
	int evaluate();
private:
	Board* board;
	lua_State* luaState;
	HashTable<EvaluationEntry> evalTable;
};