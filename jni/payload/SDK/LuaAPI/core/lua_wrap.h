#pragma once

#include "libs/Lua54/lua.hpp"
#include "../../Il2Cpp/Il2CppStructs.h"
#include <string>

namespace API
{
    extern const char* CLASS_WRAPPER_META;
    extern const char* METHOD_HANDLE_META;
    extern const char* STRUCT_WRAPPER_META;
    extern const char* ARRAY_WRAPPER_META;

    struct MethodHandle
    {
        IL2CPP::Il2CppClass* klass;
        void* instance;
        std::string name;
    };

    struct ArrayWrapper
    {
        void* arrPtr;
        IL2CPP::Il2CppClass* elementClass;
        bool elementAreValueType; // for arrays of pointers, like Main.player, Main.npc
        size_t elementSize;
    };

    struct StructWrapper
    {
        void* base;
        IL2CPP::Il2CppClass* klass;
    };

    struct ClassWrapper
    {
        IL2CPP::Il2CppClass* klass;
        void* instance;
    };

    static int methodHandle_gc(lua_State* L);
    static int methodHandle_call(lua_State* L);
    static int methodHandle_hook(lua_State* L);
    static int methodHandle_index(lua_State* L);

    void PushArrayWrapper(lua_State* L, void* arrPtr);

    static int arrayWrapper_index(lua_State* L);
    static int arrayWrapper_newindex(lua_State* L);
    static int arrayWrapper_len(lua_State* L);

    void PushStructWrapper(lua_State* L, void* base, IL2CPP::Il2CppClass* klass);

    static int structWrapper_index(lua_State* L);
    static int structWrapper_newindex(lua_State* L);

    static int classWrapper_methodCall(lua_State* L);
    static int classWrapper_ctorCall(lua_State* L);
    static int classWrapper_index(lua_State* L);
    static int classWrapper_newindex(lua_State* L);

    // picka.class
    static int lua_pickaClass(lua_State* L);

    // picka.wrap
    static int lua_pickaWrap(lua_State* L);

    void RegisterWrappers(lua_State* L);
}