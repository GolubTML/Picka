#include "LuaBridge.h"
#include "../Il2Cpp/Il2CppAPI.h"
#include "../Il2Cpp/Il2CppResolver.h"
#include "../hook.h"
#include "log.h"
#include <android/log.h>
#include <string>

namespace LuaBridge
{
    lua_State* g_Lstate = nullptr;
    uintptr_t proxy_addresses[] = 
    {
        (uintptr_t)HookHandler<0>,
        (uintptr_t)HookHandler<1>,
        (uintptr_t)HookHandler<2>,
        (uintptr_t)HookHandler<3>,
    };
    int current_slot = 0;

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

    int lua_getClass(lua_State* L)
    {
        const char* assembly = luaL_checkstring(L, 1);
        const char* namezpace = luaL_checkstring(L, 2);
        const char* klassName = luaL_checkstring(L, 3);

        void* klass = IL2CPP::Resolver::FindClass(assembly, namezpace, klassName);

        if (klass)
        {
            lua_pushlightuserdata(L, klass);
        }
        else
        {
            lua_pushnil(L);
        }

        return 1;
    }

    int lua_getFieldStatic(lua_State* L)
    {
        void* klass = nullptr;
        const char* fieldName = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            klass = lua_touserdata(L, 1);
            fieldName = luaL_checkstring(L, 2);
        }
        else
        {
            luaL_error(L, "Use picka.getFieldStatic(klass, fieldName)!");
        }

        if (!klass) return 0;

        void* fieldInfo = IL2CPP::class_get_field_from_name(klass, fieldName);
        if (!fieldInfo)
        {
            LOGI("Field not found! %s", fieldName);
            lua_pushnil(L);
            return 1;
        }

        uintptr_t value = 0;
        IL2CPP::field_static_get_value(fieldInfo, &value);

        // For test
        if (value > 0xFFFFFFFF)
            lua_pushlightuserdata(L, (void*)value);
        else
            lua_pushinteger(L, value);

        return 1;
    }

    int lua_setStaticField(lua_State* L)
    {
        if (!lua_islightuserdata(L, 1)) return luaL_error(L, "Arg 1 must be class pointer");
        void* klass = lua_touserdata(L, 1);
        const char* fieldName = luaL_checkstring(L, 2);

        void* fieldInfo = IL2CPP::class_get_field_from_name(klass, fieldName);
        if (!fieldInfo) return luaL_error(L, "Field not found!");

        uintptr_t value = 0;

        if (lua_isinteger(L, 3))
        {
            value = (uintptr_t)lua_tointeger(L, 3);
        }
        else if (lua_isboolean(L, 3))
        {
            value = (uintptr_t)lua_toboolean(L, 3);
        }
        else if (lua_islightuserdata(L, 3))
        {
            value = (uintptr_t)lua_touserdata(L, 3);
        }
        else if (lua_isnumber(L, 3))
        {
            value = (uintptr_t)lua_tonumber(L, 3);
        }

        IL2CPP::field_static_set_value(fieldInfo, &value);

        return 0;
    }

    int lua_getField(lua_State* L)
    {
        void* instance = lua_touserdata(L, 1);
        const char* fieldName = luaL_checkstring(L, 2);

        if (!instance) return 0;

        void* klass = IL2CPP::object_get_class(instance);
        void* fieldInfo = IL2CPP::class_get_field_from_name(klass, fieldName);

        if (!fieldInfo)
        {
            LOGI("--- FIELD NOT FOUND: %s ---", fieldName);
            LOGI("Listing all fields for class...");

            void* iter = nullptr;
            void* field;
            while ((field = IL2CPP::class_get_fields(klass, &iter))) 
            {
                const char* name = IL2CPP::field_get_name(field);
                LOGI("Available field: %s", name);
            }
            
            lua_pushnil(L);
            return 1;
        }

        uintptr_t value = 0;
        IL2CPP::field_get_value(instance, fieldInfo, &value);

        if (value > 0xFFFFFFFF)
            lua_pushlightuserdata(L, (void*)value);
        else
            lua_pushinteger(L, value);

        return 1;
    }

    int lua_setField(lua_State* L)
    {
        void* instance = lua_touserdata(L, 1);
        const char* fieldName = luaL_checkstring(L, 2);

        if (!instance) return 0;

        void* klass = IL2CPP::object_get_class(instance);
        void* fieldInfo = IL2CPP::class_get_field_from_name(klass, fieldName);

        if (!fieldInfo)
        {
            LOGI("Cannot find field! %s", fieldName);
            lua_pushnil(L);
            return 1;
        }

        uintptr_t value = 0;

        if (lua_isinteger(L, 3))
        {
            value = (uintptr_t)lua_tointeger(L, 3);
        }
        else if (lua_isboolean(L, 3))
        {
            value = (uintptr_t)lua_toboolean(L, 3);
        }
        else if (lua_islightuserdata(L, 3))
        {
            value = (uintptr_t)lua_touserdata(L, 3);
        }
        else if (lua_isnumber(L, 3))
        {
            value = (uintptr_t)lua_tonumber(L, 3);
        }

        IL2CPP::field_set_value(instance, fieldInfo, &value);

        return 0;
    }

    int lua_callNative(lua_State* L)
    {
        int n = lua_gettop(L);
        if (n < 1) return 0;

        uintptr_t addr = 0;
        if (lua_islightuserdata(L, 1))
        {
            addr = (uintptr_t)lua_touserdata(L, 1);
        }
        else
        {
            addr = (uintptr_t)luaL_checkinteger(L, 1);
        }

        uintptr_t args[8] = {0}; // for now, for each function, max arguments is 8.

        for (int i = 2; i <= n && (i - 2) < 8; ++i)
        {
            if (lua_isinteger(L, i)) 
            {
                args[i - 2] = (uintptr_t)lua_tointeger(L, i);
            } 
            else if (lua_isboolean(L, i)) 
            {
                args[i - 2] = (uintptr_t)lua_toboolean(L, i);
            } 
            else if (lua_islightuserdata(L, i) || lua_isuserdata(L, i)) 
            {
                args[i - 2] = (uintptr_t)lua_touserdata(L, i);
            } 
        }

        typedef uintptr_t (*GenericFn)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
        uintptr_t result = ((GenericFn)addr)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);

        lua_pushinteger(L, (lua_Integer)result);
        return 1;
    }

    int lua_hook(lua_State* L)
    {
        uintptr_t target = (uintptr_t)luaL_checkinteger(L, 1);
        int args_count = luaL_checkinteger(L, 2);

        if (current_slot >= 100)
        {
            lua_pushboolean(L, false);
            return 1;
        }

        if (!lua_isfunction(L, 3)) return luaL_error(L, "Arg 3 must be a function");

        lua_pushvalue(L, 3);
        int r = luaL_ref(L, LUA_REGISTRYINDEX);

        registeredHooks[current_slot].lua_func_ref = r;
        registeredHooks[current_slot].args_count = args_count;

        int result = hook_function(target, proxy_addresses[current_slot], &registeredHooks[current_slot].original_ptr, 4);

        if (result == 0)
        {
            current_slot++;
            lua_pushboolean(L, true);
        }
        else
        {
            lua_pushboolean(L, false);
        }

        return 1;
    }

    void RegisterAPI(lua_State* L)
    {
        g_Lstate = L;

        lua_newtable(L);

        lua_pushcfunction(L, log_print);
        lua_setfield(L, -2, "log");

        lua_pushcfunction(L, lua_newString);
        lua_setfield(L, -2, "newString");

        lua_pushcfunction(L, lua_getMethodAddr);
        lua_setfield(L, -2, "getMethodAddr");
        lua_pushcfunction(L, lua_getClass);
        lua_setfield(L, -2, "getClass");
        lua_pushcfunction(L, lua_getFieldStatic);
        lua_setfield(L, -2, "getFieldStatic");
        lua_pushcfunction(L, lua_setStaticField);
        lua_setfield(L, -2, "setFieldStatic");
        lua_pushcfunction(L, lua_getField);
        lua_setfield(L, -2, "getField");
        lua_pushcfunction(L, lua_setField);
        lua_setfield(L, -2, "setField");

        lua_pushcfunction(L, lua_callNative);
        lua_setfield(L, -2, "callNative");

        lua_pushcfunction(L, lua_hook);
        lua_setfield(L, -2, "hook");

        lua_setglobal(L, "picka");
        luaL_dostring(L, "print = picka.log");
    }
}