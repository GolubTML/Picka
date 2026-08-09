#include "LuaBridge.h"
#include "../ModLoader.h"
#include "core/lua_mod.h"
#include "core/lua_core.h"
#include "core/lua_reflection.h"
#include "core/lua_memory.h"
#include "core/lua_invoke.h"
#include "core/lua_hook.h"
#include "core/lua_wrap.h"

namespace LuaBridge
{
    lua_State* g_Lstate = nullptr;

    void RegisterAPI(lua_State* L, ModLoader* modLoader)
    {
        g_Lstate = L;
        API::g_ModLoader = modLoader;

        lua_newtable(L);

        API::RegisterCore(L);
        API::RegisterModInfo(L, modLoader);
        API::RegisterReflection(L);
        API::RegisterMemory(L);
        API::RegisterInvoker(L);
        API::RegisterHook(L);
        API::RegisterWrappers(L);

        lua_setglobal(L, "picka");
        luaL_dostring(L, "print = picka.log");
    }
}