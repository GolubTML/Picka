#pragma once

#include "libs/Lua54/lua.hpp"

namespace API
{
    static int log_print(lua_State* L);
    
    static int lua_newString(lua_State* L);
    static int lua_loadTexture(lua_State* L);

    void RegisterCore(lua_State* L);
}   