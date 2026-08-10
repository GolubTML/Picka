#pragma once

#include "libs/Lua54/lua.hpp"

namespace Hook
{
    extern uintptr_t* proxy_addresses;
    extern int current_slot;

    int RegisterHook(lua_State* L, uintptr_t target, int argsCount, int callbackStackIdx);
}

namespace API
{
    static int lua_hook(lua_State* L);

    void RegisterHook(lua_State* L);
}