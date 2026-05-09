#include "LuaHook.h"
#include "LuaBridge.h"
#include "log.h"

namespace LuaBridge
{
    LuaHook registeredHooks[MAX_HOOKS];

    uintptr_t DummyNative(uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, 
                         uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7) 
    {
        return 0;
    }

    uintptr_t Proxy(int slot_id, uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3, uintptr_t x4, uintptr_t x5, uintptr_t x6, uintptr_t x7)
    {
        LuaHook& hook = registeredHooks[slot_id];
        uintptr_t lastResult = 0;

        if (hook.callbacks.empty())
        {
            typedef uintptr_t (*OrigFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
            return ((OrigFn)hook.original_ptr)(x0, x1, x2, x3, x4, x5, x6, x7);
        }

        int callIndex = 0;

        for (int callback_ref : hook.callbacks)
        {
            lua_rawgeti(g_Lstate, LUA_REGISTRYINDEX, callback_ref);

            if (!lua_isfunction(g_Lstate, -1)) 
            {
                LOGI("Critical Error: Slot %d, Ref %d is TYPE %s (not a function!)", 
                    slot_id, callback_ref, lua_typename(g_Lstate, lua_type(g_Lstate, -1)));
                lua_pop(g_Lstate, 1);
                
                // typedef uintptr_t (*OrigFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
                // return ((OrigFn)hook.original_ptr)(x0, x1, x2, x3);
            }

            uintptr_t original = (callIndex == 0) ? hook.original_ptr : (uintptr_t)DummyNative;

            lua_pushlightuserdata(g_Lstate, (void*)original);
            lua_pushinteger(g_Lstate, x0); 
            lua_pushinteger(g_Lstate, x1);
            lua_pushinteger(g_Lstate, x2);
            lua_pushinteger(g_Lstate, x3);
            lua_pushinteger(g_Lstate, x4); 
            lua_pushinteger(g_Lstate, x5);
            lua_pushinteger(g_Lstate, x6);
            lua_pushinteger(g_Lstate, x7);

            if (lua_pcall(g_Lstate, 9, 1, 0) == LUA_OK) 
            {
                if (!lua_isnil(g_Lstate, -1)) 
                {
                    lastResult = (uintptr_t)lua_tointeger(g_Lstate, -1);
                }
                lua_pop(g_Lstate, 1);
            }
            else
            {
                LOGI("Lua Error in slot %d: %s", slot_id, lua_tostring(g_Lstate, -1));
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

        LOGI("Cleared all hooks!");
    }
}