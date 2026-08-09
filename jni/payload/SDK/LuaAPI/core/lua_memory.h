#pragma once

#include "libs/Lua54/lua.hpp"

namespace API
{
    static int lua_getArrayLength(lua_State* L);
    static int lua_getArrayElement(lua_State* L);

    static int lua_readFloat(lua_State* L); // for structure, not for class

    void RegisterMemory(lua_State* L);
}