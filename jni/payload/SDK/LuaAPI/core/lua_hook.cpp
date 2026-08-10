#include "lua_hook.h"

#include "../LuaHook.h"
#include "../../hook.h"
#include "../../../log.h"
#include <map>

namespace Hook
{
    static std::map<uintptr_t, int> addrToSlot;

    template<std::size_t... Is>
    constexpr auto make_proxy_addresses(std::index_sequence<Is...>) 
    {
        return std::array<uintptr_t, sizeof...(Is)>{ (uintptr_t)LuaBridge::HookHandler<Is>... };
    }

    static auto proxy_array = make_proxy_addresses(std::make_index_sequence<MAX_HOOKS>{}); // make 256 free hooks at once
    uintptr_t* proxy_addresses = proxy_array.data();

    int current_slot = 0;

    int RegisterHook(lua_State* L, uintptr_t target, int argsCount, int callbackStackIdx)
    {
        if (addrToSlot.count(target) > 0)
        {
            int slot = addrToSlot[target];

            lua_pushvalue(L, 3);
            int callback = luaL_ref(L, LUA_REGISTRYINDEX);
            
            LuaBridge::registeredHooks[slot].callbacks.push_back(callback);
            M_LOGI("Hook updated: Mod added to chain in slot %d. Total mods: %zu", slot, LuaBridge::registeredHooks[slot].callbacks.size());

            lua_pushboolean(L, true);
            return 1;
        }

        if (current_slot >= MAX_HOOKS)
        {
            lua_pushboolean(L, false);
            return 1;
        }

        if (!lua_isfunction(L, 3)) return luaL_error(L, "Arg 3 must be a function");

        lua_pushvalue(L, 3);
        int r = luaL_ref(L, LUA_REGISTRYINDEX);

        LuaBridge::registeredHooks[current_slot].callbacks.push_back(r);
        LuaBridge::registeredHooks[current_slot].args_count = argsCount;

        int result = hook_function(target, proxy_addresses[current_slot], &LuaBridge::registeredHooks[current_slot].original_ptr, 4);

        if (result == 0)
        {
            addrToSlot[target] = current_slot;
            current_slot++;
            lua_pushboolean(L, true);
        }
        else
        {
            lua_pushboolean(L, false);
        }

        return 1;
    }
}

namespace API
{
    int lua_hook(lua_State* L)
    {
        uintptr_t target = (uintptr_t)luaL_checkinteger(L, 1);
        int argsCount = luaL_checkinteger(L, 2);

        if (!lua_isfunction(L, 3)) luaL_error(L, "Argument 3 must be a funtion!");
            
        return Hook::RegisterHook(L, target, argsCount, 3);
    }

    void RegisterHook(lua_State* L)
    {
        lua_pushcfunction(L, lua_hook);
        lua_setfield(L, -2, "hook");
    }
}