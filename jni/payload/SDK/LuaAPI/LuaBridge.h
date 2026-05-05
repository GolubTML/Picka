#pragma once

#include "libs/Lua54/lua.hpp"

namespace LuaBridge
{
    static int log_print(lua_State* L);
    static int lua_getMethodAddr(lua_State* L);

    void RegisterAPI(lua_State* L);
}