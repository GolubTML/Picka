#pragma once

#include "libs/Lua54/lua.hpp"
#include "../../Il2Cpp/Il2CppStructs.h"
#include <functional>

namespace Invoke
{
    uintptr_t CallMethod(lua_State* L, IL2CPP::MethodInfo* methodInfo, int argsBase);
    // this function is imitates Lua stack, but we will use it on C++ side 
    uintptr_t CallMethodInternal(lua_State* L, IL2CPP::MethodInfo* methodInfo, std::function<void(lua_State*)> pushArgs);
}

namespace API
{
    static int lua_callNative(lua_State* L);
    static int lua_callMethod(lua_State* L); // like callNative, but takes MethodInfo. And, uses specific arguments type

    void RegisterInvoker(lua_State* L);
}