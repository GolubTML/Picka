#pragma once

#include "libs/Lua54/lua.hpp"

namespace API
{
    extern uintptr_t* proxy_addresses;
    extern int current_slot;

    static int lua_hook(lua_State* L);

    void RegisterHook(lua_State* L);
}