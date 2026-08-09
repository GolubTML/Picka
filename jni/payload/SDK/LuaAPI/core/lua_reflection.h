#pragma once

#include "libs/Lua54/lua.hpp"

namespace API
{
    static int lua_getMethodAddr(lua_State* L);
    static int lua_getMethodInfo(lua_State* L); // the same as lua_getMethodAddr, but we get MethodInfo now
    
    static int lua_getClass(lua_State* L);
    static int lua_getStaticField(lua_State* L);
    static int lua_setStaticField(lua_State* L);

    // this methods for instances
    static int lua_getField(lua_State* L);  
    static int lua_setField(lua_State* L);
    static int lua_getFieldOffset(lua_State* L);

    void RegisterReflection(lua_State* L);
}