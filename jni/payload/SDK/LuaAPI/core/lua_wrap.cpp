#include "lua_wrap.h"

#include "../../../log.h"
#include "../../Il2Cpp/Il2CppResolver.h"
#include "../LuaHelper.h"
#include "lua_reflection.h"
#include "lua_invoke.h"
#include "lua_hook.h"

#include <unordered_map>

namespace API
{
    static const char* CLASS_WRAPPER_META = "picka.ClassWrapper";
    static const char* METHOD_HANDLE_META = "picka.MethodHandle";

    std::unordered_map<IL2CPP::Il2CppClass*, std::unordered_map<std::string, IL2CPP::MethodInfo*>> g_MethodCache;
    std::unordered_map<IL2CPP::Il2CppClass*, std::unordered_map<std::string, void*>> g_FieldCache;

    static std::string MakeMethodKey(const char* name, int argc)
    {
        return std::string(name) + "#" + std::to_string(argc);
    }

    static IL2CPP::MethodInfo* cacheMethodInfo(IL2CPP::Il2CppClass* klass, const char* name, int argc)
    {
        auto& classCache = g_MethodCache[klass];
        std::string key = MakeMethodKey(name, argc);

        auto it = classCache.find(key);
        if (it != classCache.end())
        {
            // LOGI("Cache HIT: %s", key.c_str());
            return it->second;
        }

        // LOGI("Cache MISS: %s", key.c_str());
        IL2CPP::MethodInfo* method = Reflections::FindMethod(klass, name, argc);
        classCache[key] = method;

        return method;
    }

    static void* cacheFieldInfo(IL2CPP::Il2CppClass* klass, const char* name, bool isStatic)
    {
        auto& classCache = g_FieldCache[klass];
        std::string key = std::string(name) + (isStatic ? "#s" : "#i");

        auto it = classCache.find(key);
        if (it != classCache.end())
        {
            // LOGI("Cache HIT: %s", key.c_str());
            return it->second;
        }

        // LOGI("Cache MISS: %s", key.c_str());    
        void* field = Reflections::FindField(klass, name, isStatic);
        classCache[key] = field;

        return field;
    }

    // for SomeClass.SomeMethod:hook(function() end)
    struct MethodHandle
    {
        IL2CPP::Il2CppClass* klass;
        void* instance;
        std::string name;
    };

    int methodHandle_gc(lua_State* L)
    {
        MethodHandle* h = (MethodHandle*)luaL_checkudata(L, 1, METHOD_HANDLE_META);
        h->~MethodHandle();

        return 0;
    }
    int methodHandle_call(lua_State* L)
    {
        MethodHandle* h = (MethodHandle*)luaL_checkudata(L, 1, METHOD_HANDLE_META);

        int argc = lua_gettop(L) - 1; // we are not including 'instance' in function
        IL2CPP::MethodInfo* method = cacheMethodInfo(h->klass, h->name.c_str(), argc);

        if (!method)
            return luaL_error(L, "Method '%s' with %d args not found", h->name.c_str(), argc);

        bool isStatic = (method->flags & 0x0010);

        lua_remove(L, 1);

        if (!isStatic)
        {
            lua_pushlightuserdata(L, h->instance);
            lua_insert(L, 1);
        }

        uintptr_t result = Invoke::CallMethod(L, method, 1);
        lua_pushinteger(L, (lua_Integer)result);

        return 1;
    }
    int methodHandle_hook(lua_State* L)
    {
        MethodHandle* h = (MethodHandle*)luaL_checkudata(L, 1, METHOD_HANDLE_META);

        if (!lua_isfunction(L, 2)) return luaL_error(L, "Method hook() expected a function!");

        IL2CPP::MethodInfo* method = cacheMethodInfo(h->klass, h->name.c_str(), -1);

        if (!method)
            return luaL_error(L, "Cannot resolve method '%s' for hook", h->name.c_str());

        uintptr_t addr = IL2CPP::Resolver::GetMethodPtr(method);
        if (!addr)
            return luaL_error(L, "Cannot get method pointer for '%s'", h->name.c_str());

        return Hook::RegisterHook(L, addr, -1, 2);
    }

    int methodHandle_index(lua_State* L)
    {
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "hook") == 0)
        {
            lua_pushcfunction(L, methodHandle_hook);
            return 1;
        }

        // I need to write tests for overloads, before i make hook overload
        
        // if (strcmp(key, "hookOverload") == 0)
        // {
        //     lua_pushcfunction(L, ...); 
        //     return 1;
        // }

        lua_pushnil(L);
        return 1;
    }

    struct ClassWrapper
    {
        IL2CPP::Il2CppClass* klass;
        void* instance;
    };

    int classWrapper_methodCall(lua_State* L)
    {
        IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));
        void* instance = lua_touserdata(L, lua_upvalueindex(2));
        const char* methodName = lua_tostring(L, lua_upvalueindex(3));

        int argc = lua_gettop(L);

        IL2CPP::MethodInfo* methodInfo = cacheMethodInfo(klass, methodName, argc);
        if (!methodInfo)
            return luaL_error(L, "Method '%s' with %d args not found", methodName, argc);

        bool isStatic = (methodInfo->flags & 0x0010);

        if (!isStatic)
        {
            lua_pushlightuserdata(L, instance);
            lua_insert(L, 1);
        }

        uintptr_t result = Invoke::CallMethod(L, methodInfo, 1);
        lua_pushinteger(L, (lua_Integer)result);

        return 1;
    }
    int classWrapper_index(lua_State* L)
    {
        ClassWrapper* w = (ClassWrapper*)luaL_checkudata(L, 1, CLASS_WRAPPER_META);
        const char* fieldName = luaL_checkstring(L, 2);

        bool isStatic = (w->instance == nullptr);
        IL2CPP::Il2CppClass* klass = w->instance ? IL2CPP::object_get_class(w->instance) : w->klass;

        void* fieldInfo = cacheFieldInfo(klass, fieldName, isStatic);
        if (fieldInfo)
        {
            uintptr_t value = Reflections::GetFieldValue(w->instance, fieldInfo, isStatic);
            LuaBridge::Helper::luaPushUintptr(L, value);

            return 1;
        }

        void* mem = lua_newuserdata(L, sizeof(MethodHandle));
        MethodHandle* h = new (mem) MethodHandle();
        h->klass = w->klass;
        h->instance = w->instance;
        h->name = fieldName;

        // lua_pushlightuserdata(L, w->klass);
        // lua_pushlightuserdata(L, w->instance);
        // lua_pushstring(L, fieldName);
        // lua_pushcclosure(L, classWrapper_methodCall, 3);

        luaL_getmetatable(L, METHOD_HANDLE_META);
        lua_setmetatable(L, -2);

        return 1;
    }
    int classWrapper_newindex(lua_State* L)
    {
        ClassWrapper* w = (ClassWrapper*)luaL_checkudata(L, 1, CLASS_WRAPPER_META);
        const char* fieldName = luaL_checkstring(L, 2);

        bool isStatic = (w->instance == nullptr);
        IL2CPP::Il2CppClass* klass = w->instance ? IL2CPP::object_get_class(w->instance) : w->klass;

        void* fieldInfo = cacheFieldInfo(klass, fieldName, isStatic);
        if (!fieldInfo)
            return luaL_error(L, "Field %s not found!", fieldName);

        uintptr_t value = LuaBridge::Helper::luaToUintptr(L, 3);
        Reflections::SetFieldValue(w->instance, fieldInfo, value, isStatic);

        return 0;
    }

    int lua_pickaClass(lua_State* L)
    {
        const char* namezpace = luaL_checkstring(L, 1);
        const char* klassName = luaL_checkstring(L, 2);
        const char* assembly = luaL_optstring(L, 3, "Assembly-CSharp");

        IL2CPP::Il2CppClass* klass = IL2CPP::Resolver::FindClass(assembly, namezpace, klassName);
        if (!klass)
        {
            lua_pushnil(L);
            return 1;
        }

        ClassWrapper* w = (ClassWrapper*)lua_newuserdata(L, sizeof(ClassWrapper));
        w->klass = klass;
        w->instance = nullptr;

        luaL_getmetatable(L, CLASS_WRAPPER_META);
        lua_setmetatable(L, -2);

        return 1;
    }

    int lua_pickaWrap(lua_State* L)
    {
        void* instance = nullptr;
        if (lua_islightuserdata(L, 1)) instance = lua_touserdata(L, 1);
        else if (lua_isnumber(L, 1)) instance = (void*)(uintptr_t)lua_tointeger(L, 1);

        if (!instance) 
        { 
            lua_pushnil(L); 
            return 1; 
        }

        IL2CPP::Il2CppClass* klass = IL2CPP::object_get_class(instance);
        if (!klass)
        {
            lua_pushnil(L);
            return 1;
        }

        ClassWrapper* w = (ClassWrapper*)lua_newuserdata(L, sizeof(ClassWrapper));
        w->klass = klass;
        w->instance = instance;

        luaL_getmetatable(L, CLASS_WRAPPER_META);
        lua_setmetatable(L, -2);

        return 1;
    }

    void RegisterWrappers(lua_State* L)
    {
        luaL_newmetatable(L, CLASS_WRAPPER_META);
        lua_pushcfunction(L, classWrapper_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, classWrapper_newindex);
        lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);

        luaL_newmetatable(L, METHOD_HANDLE_META);
        lua_pushcfunction(L, methodHandle_call);
        lua_setfield(L, -2, "__call");
        lua_pushcfunction(L, methodHandle_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, methodHandle_gc);
        lua_setfield(L, -2, "__gc");
        lua_pop(L, 1);

        lua_pushcfunction(L, lua_pickaClass);
        lua_setfield(L, -2, "class");
        lua_pushcfunction(L, lua_pickaWrap);
        lua_setfield(L, -2, "wrap");
    }
}