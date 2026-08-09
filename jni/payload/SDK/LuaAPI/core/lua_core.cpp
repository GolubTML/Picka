#include "lua_core.h"
#include "../../log.h"
#include "../../Il2Cpp/Il2CppResolver.h"

#include <string>

namespace API
{
    int log_print(lua_State* L)
    {
        int n = lua_gettop(L);
        std::string out = "[Lua] ";

        for (int i = 1; i <= n; i++) 
        {
            size_t len;
            const char* s = luaL_tolstring(L, i, &len);
            if (s) out += s;
            if (i < n) out += "  ";
            lua_pop(L, 1);
        }

        M_LOGI("%s", out.c_str());

        return 0;
    }

    int lua_newString(lua_State* L)
    {
        const char* str = luaL_checkstring(L, 1);

        void* il2cpp_str = IL2CPP::new_string(str);

        if (il2cpp_str)
        {
            lua_pushlightuserdata(L, il2cpp_str);
        }
        else
        {
            lua_pushnil(L);
        }

        return 1;
    }

    void RegisterCore(lua_State* L)
    {
        lua_pushcfunction(L, log_print);
        lua_setfield(L, -2, "log");

        lua_pushcfunction(L, lua_newString);
        lua_setfield(L, -2, "newString");
    }
}