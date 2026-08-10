#pragma once

#include "libs/Lua54/lua.hpp"
#include "../Il2Cpp/Il2CppStructs.h"

namespace LuaBridge::Helper
{
    void setTypedValue(lua_State* L,  int luaValueIdx, void* addr, const IL2CPP::Il2CppType* fieldType);
    void fillStructFromTable(lua_State* L, int tableIdx, IL2CPP::Il2CppClass* klass, void* buffer);

    uintptr_t luaToUintptr(lua_State* L, int idx);
    void luaPushUintptr(lua_State* L, uintptr_t value);
}