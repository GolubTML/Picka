#include "LuaHook.h"
#include "LuaBridge.h"
#include "log.h"

namespace LuaBridge
{
    LuaHook registeredHooks[MAX_HOOKS];

    uintptr_t DummyNative(uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, 
        uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7,
        uintptr_t x8, uintptr_t x9, uintptr_t x10, uintptr_t x11,
        uintptr_t x12, uintptr_t x13, uintptr_t x14, uintptr_t x15) 
    {
        return 0;
    }

    uintptr_t Proxy(int slot_id, uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, 
        uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7,
        uintptr_t x8, uintptr_t x9, uintptr_t x10, uintptr_t x11,
        uintptr_t x12, uintptr_t x13, uintptr_t x14, uintptr_t x15)
    {
        LuaHook& hook = registeredHooks[slot_id];
        uintptr_t lastResult = 0;

        if (hook.callbacks.empty())
        {
            typedef uintptr_t (*OrigFn)(
                uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t,
                uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
            return ((OrigFn)hook.original_ptr)(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
        }

        int callIndex = 0;

        for (int callback_ref : hook.callbacks)
        {
            lua_rawgeti(g_Lstate, LUA_REGISTRYINDEX, callback_ref);

            if (!lua_isfunction(g_Lstate, -1)) 
            {
                M_LOGE("Critical Error: Slot %d, Ref %d is TYPE %s (not a function!)", 
                    slot_id, callback_ref, lua_typename(g_Lstate, lua_type(g_Lstate, -1)));
                lua_pop(g_Lstate, 1);
            }

            uintptr_t original = (callIndex == 0) ? hook.original_ptr : (uintptr_t)DummyNative;

            lua_pushlightuserdata(g_Lstate, (void*)original);

            uintptr_t args[] = {x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15};
            for(int i = 0; i < 16; i++) 
            {
                if (args[i] > 0xFFFFFFFF) lua_pushlightuserdata(g_Lstate, (void*)args[i]);
                else lua_pushinteger(g_Lstate, (lua_Integer)args[i]);
            }

            if (lua_pcall(g_Lstate, 17, 1, 0) == LUA_OK) 
            {
                if (!lua_isnil(g_Lstate, -1)) 
                {
                    lastResult = (uintptr_t)lua_tointeger(g_Lstate, -1);
                }
                lua_pop(g_Lstate, 1);
            }
            else
            {
                M_LOGI("Lua Error in slot %d: %s", slot_id, lua_tostring(g_Lstate, -1));
                lua_pop(g_Lstate, 1);
            }

            // uintptr_t result = (uintptr_t)lua_tointeger(g_Lstate, -1);
            // lua_pop(g_Lstate, 1);

            // LOGI("Proxy called for slot: %d, func_ref: %d", slot_id, hook.lua_func_ref);
            callIndex++;
        }
        
        return lastResult;
    }

    void ClearAllHooks()
    {
        for (int i = 0; i < MAX_HOOKS; ++i)
        {
            registeredHooks[i].callbacks.clear();
        }

        M_LOGI("Cleared all hooks!");
    }
}