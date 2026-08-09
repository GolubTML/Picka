#pragma once

#include "libs/Lua54/lua.hpp"

namespace API
{
    static int classWrapper_methodCall(lua_State* L);
    static int classWrapper_index(lua_State* L);
    static int classWrapper_newindex(lua_State* L);

    // picka.class
    static int lua_pickaClass(lua_State* L);

    // picka.wrap
    static int lua_pickaWrap(lua_State* L);

    void RegisterWrappers(lua_State* L);
}