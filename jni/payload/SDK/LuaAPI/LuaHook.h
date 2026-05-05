#pragma once
#include <cstdint>

namespace LuaBridge
{
    struct LuaHook
    {
        int lua_func_ref = -1;
        uintptr_t original_ptr = 0;
        int args_count = 0;
    };
    
    extern LuaHook registeredHooks[100];

    uintptr_t Proxy(int slot_id, uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3);

    template<int ID>
    uintptr_t HookHandler(uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, 
                      uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7)
    {
        return Proxy(ID, x0, x1, x2, x3);
    }
}