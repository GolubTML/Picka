#pragma once
#include <cstdint>
#include <vector>

#define MAX_HOOKS 256

namespace LuaBridge
{
    struct LuaHook
    {
        std::vector<int> callbacks;
        uintptr_t original_ptr = 0;
        int args_count = 0;
    };
    
    extern LuaHook registeredHooks[MAX_HOOKS];

    uintptr_t Proxy(int slot_id, uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, 
        uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7,
        uintptr_t x8, uintptr_t x9, uintptr_t x10, uintptr_t x11,
        uintptr_t x12, uintptr_t x13, uintptr_t x14, uintptr_t x15); // it should be enought now

    template<int ID>
    uintptr_t HookHandler(uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, 
        uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7,
        uintptr_t x8, uintptr_t x9, uintptr_t x10, uintptr_t x11,
        uintptr_t x12, uintptr_t x13, uintptr_t x14, uintptr_t x15)
    {
        return Proxy(ID, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
    }

    void ClearAllHooks();
}