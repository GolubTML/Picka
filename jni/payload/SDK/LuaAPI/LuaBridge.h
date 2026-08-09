#pragma once

#include "libs/Lua54/lua.hpp"

class ModLoader;

namespace LuaBridge
{
    extern lua_State* g_Lstate;

    void RegisterAPI(lua_State* L, ModLoader* modLoader);
}