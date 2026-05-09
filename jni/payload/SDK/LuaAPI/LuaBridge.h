#pragma once

#include "libs/Lua54/lua.hpp"
#include "LuaHook.h"

namespace LuaBridge
{
    extern lua_State* g_Lstate;
    extern uintptr_t* proxy_addresses;
    extern int current_slot;

    static int log_print(lua_State* L);
    
    static int lua_newString(lua_State* L);

    static int lua_getMethodAddr(lua_State* L);
    
    static int lua_getClass(lua_State* L);
    static int lua_getStaticField(lua_State* L);
    static int lua_setStaticField(lua_State* L);

    // this methods for instances
    static int lua_getField(lua_State* L);
    static int lua_setField(lua_State* L);

    static int lua_getArrayLength(lua_State* L);
    static int lua_getArrayElement(lua_State* L);

    static int lua_callNative(lua_State* L);
    static int lua_hook(lua_State* L);

    void RegisterAPI(lua_State* L);
}