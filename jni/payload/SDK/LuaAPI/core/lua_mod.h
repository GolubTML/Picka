#pragma once

#include "libs/Lua54/lua.hpp"

class ModLoader;

namespace API
{
    extern ModLoader* g_ModLoader;
    ModLoader* getModLoader();

    static int lua_getModName(lua_State* L);
    static int lua_getModAuthor(lua_State* L);
    static int lua_getModVersion(lua_State* L);
    static int lua_getModInfo(lua_State* L);

    void RegisterModInfo(lua_State* L, ModLoader* modLoader);
}