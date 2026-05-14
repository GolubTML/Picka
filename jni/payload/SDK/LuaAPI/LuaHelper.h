#pragma once

#include "libs/Lua54/lua.hpp"
#include "../Il2Cpp/Il2CppStructs.h"

namespace LuaBridge::Helper
{
    // in future will be more functions
    void fillStructFromTable(lua_State* L, int tableIdx, IL2CPP::Il2CppClass* klass, void* buffer);
}