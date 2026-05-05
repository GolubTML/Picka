#include "LuaHook.h"
#include "LuaBridge.h"
#include "log.h"

namespace LuaBridge
{
    LuaHook registeredHooks[100];

    uintptr_t Proxy(int slot_id, uintptr_t x0, uintptr_t x1, uintptr_t x2, uintptr_t x3)
    {
        LuaHook& hook = registeredHooks[slot_id];

        lua_rawgeti(g_Lstate, LUA_REGISTRYINDEX, hook.lua_func_ref);

        if (!lua_isfunction(g_Lstate, -1)) 
        {
            LOGI("Critical Error: Slot %d, Ref %d is TYPE %s (not a function!)", 
                slot_id, hook.lua_func_ref, lua_typename(g_Lstate, lua_type(g_Lstate, -1)));
            lua_pop(g_Lstate, 1);
            
            typedef uintptr_t (*OrigFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
            return ((OrigFn)hook.original_ptr)(x0, x1, x2, x3);
        }

        lua_pushlightuserdata(g_Lstate, (void*)hook.original_ptr);
        lua_pushinteger(g_Lstate, x0); 
        lua_pushinteger(g_Lstate, x1);
        lua_pushinteger(g_Lstate, x2);
        lua_pushinteger(g_Lstate, x3);

        if (lua_pcall(g_Lstate, 5, 1, 0) != LUA_OK) 
        {
            LOGI("Lua Hook Error in slot %d: %s", slot_id, lua_tostring(g_Lstate, -1));
            lua_pop(g_Lstate, 1);
            typedef uintptr_t (*OrigFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
            return ((OrigFn)hook.original_ptr)(x0, x1, x2, x3);
        }

        uintptr_t result = (uintptr_t)lua_tointeger(g_Lstate, -1);
        lua_pop(g_Lstate, 1);

        LOGI("Proxy called for slot: %d, func_ref: %d", slot_id, hook.lua_func_ref);

        return result;
    }
}