#include "LuaBridge.h"
#include "../Il2Cpp/Il2CppAPI.h"
#include "../Il2Cpp/Il2CppResolver.h"
#include <android/log.h>
#include <string>

namespace LuaBridge
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

        __android_log_print(ANDROID_LOG_INFO, "Payload", "%s", out.c_str());

        return 0;
    }

    int lua_getMethodAddr(lua_State* L)
    {
        const char* assembly = luaL_checkstring(L, 1);
        const char* namezpace = luaL_checkstring(L, 2);
        const char* klass = luaL_checkstring(L, 3);
        const char* method = luaL_checkstring(L, 4);
        int args = luaL_checkinteger(L, 5);

        void* methodInfo = IL2CPP::Resolver::FindMethod(assembly, namezpace, klass, method, args);
        uintptr_t addr = IL2CPP::Resolver::GetMethodPtr(methodInfo);

        if (addr)
        {
            lua_pushinteger(L, addr);
        }
        else
        {
            lua_pushnil(L);
        }

        return 1;
    }

    void RegisterAPI(lua_State* L)
    {
        lua_newtable(L);

        lua_pushcfunction(L, log_print);
        lua_setfield(L, -2, "log");

        lua_pushcfunction(L, lua_getMethodAddr);
        lua_setfield(L, -2, "getMethodAddr");

        lua_setglobal(L, "picka");
        luaL_dostring(L, "print = picka.log");
    }
}