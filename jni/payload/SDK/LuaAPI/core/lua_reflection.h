#pragma once

#include "libs/Lua54/lua.hpp"
#include "../../Il2Cpp/Il2CppStructs.h"

// here will be all methods, which we will use not only here
namespace Reflections
{
    IL2CPP::MethodInfo* FindMethod(IL2CPP::Il2CppClass* klass, const char* name, int argc);

    void* FindField(IL2CPP::Il2CppClass* klass, const char* fieldName, bool isStatic);

    uintptr_t GetFieldValue(void* instance, void* fieldInfo, bool isStatic);
    void SetFieldValue(void* instance, void* fieldInfo, uintptr_t value, bool isStatic);

    void PushTypedValue(lua_State* L, void* addr, const IL2CPP::Il2CppType* fieldType, IL2CPP::Il2CppClass* fieldClassIfStruct);
}

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