#include "LuaBridge.h"
#include <android/log.h>
#include <string>

namespace LuaBridge
{
    static int log_print(lua_State* L)
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

        __android_log_print(ANDROID_LOG_INFO, "Payload", "%s", out.c_str());

        return 0;
    }

    void RegisterAPI(lua_State* L)
    {
        lua_newtable(L);

        lua_pushcfunction(L, log_print);
        lua_setfield(L, -2, "log");

        lua_setglobal(L, "picka");
        luaL_dostring(L, "print = picka.log");
    }
}