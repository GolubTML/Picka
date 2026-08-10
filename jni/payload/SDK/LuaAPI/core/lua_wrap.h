#pragma once

#include "libs/Lua54/lua.hpp"
#include "../../Il2Cpp/Il2CppStructs.h"

namespace API
{
    static int methodHandle_gc(lua_State* L);
    static int methodHandle_call(lua_State* L);
    static int methodHandle_hook(lua_State* L);
    static int methodHandle_index(lua_State* L);

    void PushStructWrapper(lua_State* L, void* base, IL2CPP::Il2CppClass* klass);

    static int structWrapper_index(lua_State* L);
    static int structWrapper_newindex(lua_State* L);

    static int classWrapper_methodCall(lua_State* L);
    static int classWrapper_index(lua_State* L);
    static int classWrapper_newindex(lua_State* L);

    // picka.class
    static int lua_pickaClass(lua_State* L);

    // picka.wrap
    static int lua_pickaWrap(lua_State* L);

    void RegisterWrappers(lua_State* L);
}