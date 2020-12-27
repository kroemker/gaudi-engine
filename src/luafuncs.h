#pragma once
#include <lua.hpp>
#include "Board.h"

void setup_lua(Board* b);

int l_getPieceCount(lua_State * L);
int l_getPieceInfo(lua_State * L);
int l_getMoveCount(lua_State* L);
int l_getMoveInfo(lua_State* L);
int l_getFileRank(lua_State* L);
int l_getBoard(lua_State* L);