#pragma once

#include "libs/Lua54/lua.hpp"
#include "LuaHook.h"

class ModLoader;

namespace LuaBridge
{
    extern lua_State* g_Lstate;
    extern uintptr_t* proxy_addresses;
    extern int current_slot;

    static int log_print(lua_State* L);
    
    static int lua_newString(lua_State* L);

    static int lua_getModName(lua_State* L);

    static int lua_getMethodAddr(lua_State* L);
    static int lua_getMethodInfo(lua_State* L); // the same as lua_getMethodAddr, but we get MethodInfo now
    
    static int lua_getClass(lua_State* L);
    static int lua_getStaticField(lua_State* L);
    static int lua_setStaticField(lua_State* L);

    // this methods for instances
    static int lua_getField(lua_State* L);  
    static int lua_setField(lua_State* L);
    static int lua_getFieldOffset(lua_State* L);

    static int lua_getArrayLength(lua_State* L);
    static int lua_getArrayElement(lua_State* L);

    static int lua_readFloat(lua_State* L); // for structure, not for class

    static int lua_callNative(lua_State* L);
    static int lua_callMethod(lua_State* L); // like callNative, but takes MethodInfo. And, uses specific arguments type
    static int lua_hook(lua_State* L);

    void RegisterAPI(lua_State* L, ModLoader* modLoader);
}