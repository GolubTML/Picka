#include "lua_reflection.h"

#include "../../Il2Cpp/Il2CppResolver.h"
#include "../LuaHelper.h"
#include "../../../log.h"
#include "lua_wrap.h"

namespace Reflections
{
    IL2CPP::MethodInfo* FindMethod(IL2CPP::Il2CppClass* klass, const char* name, int argc)
    {
        if (!klass || !name) return nullptr;
        return IL2CPP::class_get_method_from_name(klass, name, argc);
    }

    void* FindField(IL2CPP::Il2CppClass* klass, const char* fieldName, bool isStatic)
    {
        if (!klass || !fieldName) return nullptr;
        return isStatic ? IL2CPP::Resolver::FindStaticField(klass, fieldName) : IL2CPP::Resolver::FindField(klass, fieldName);
    }

    uintptr_t GetFieldValue(void* instance, void* fieldInfo, bool isStatic)
    {
        if (!fieldInfo) return 0;

        uintptr_t value = 0;
        if (isStatic)
            IL2CPP::field_static_get_value(fieldInfo, &value);
        else
            IL2CPP::field_get_value(instance, fieldInfo, &value);

        return value;
    }
    void SetFieldValue(void* instance, void* fieldInfo, uintptr_t value, bool isStatic)
    {
        if (!fieldInfo) return;

        if (isStatic)
            IL2CPP::field_static_set_value(fieldInfo, &value);
        else
            IL2CPP::field_set_value(instance, fieldInfo, &value);
    }

    void PushTypedValue(lua_State* L, void* addr, const IL2CPP::Il2CppType* fieldType, IL2CPP::Il2CppClass* fieldClassIfStruct)
    {
        uint8_t type_enum = IL2CPP::type_get_type(fieldType);

        switch (type_enum)
        {
            case 0x02: 
                lua_pushboolean(L, *(bool*)addr);
                return;
            case 0x0C: 
                lua_pushnumber(L, *(float*)addr);
                return;
            case 0x03: 
            case 0x08:
                lua_pushinteger(L, *(int32_t*)addr);
                return;
            case 0x05: 
                lua_pushinteger(L, *(uint8_t*)addr);
                return;
            case 0x0E:
            {
                void* il2cppStr = *(void**)addr;
                if (!il2cppStr) { lua_pushnil(L); return; }

                std::string str = IL2CPP::Resolver::GetString(il2cppStr);
                lua_pushstring(L, str.c_str());
                return;
            }
            default:
                break;
        }

        if (fieldClassIfStruct && IL2CPP::class_is_valuetype(fieldClassIfStruct))
        {
            API::PushStructWrapper(L, addr, fieldClassIfStruct);
            return;
        }

        uintptr_t raw = *(uintptr_t*)addr;
        LuaBridge::Helper::luaPushUintptr(L, raw);
    }
}

namespace API
{
    int lua_getMethodAddr(lua_State* L)
    {
        IL2CPP::MethodInfo* methodInfo = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            // if we want write picka.getMethodPtr(klass, methodName, argsCount)
            IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
            const char* methodName = luaL_checkstring(L, 2);
            int argc = luaL_checkinteger(L, 3);

            methodInfo = Reflections::FindMethod(klass, methodName, argc);
        }
        else
        {
            const char* assembly = luaL_checkstring(L, 1);
            const char* namezpace = luaL_checkstring(L, 2);
            const char* klass = luaL_checkstring(L, 3);
            const char* method = luaL_checkstring(L, 4);
            int argc = luaL_checkinteger(L, 5);

            methodInfo = IL2CPP::Resolver::FindMethod(assembly, namezpace, klass, method, argc);
        }

        if (methodInfo)
        {
            uintptr_t addr = IL2CPP::Resolver::GetMethodPtr(methodInfo);
            lua_pushinteger(L, addr);
        }
        else
        {
            lua_pushnil(L);
        }
        
        return 1;
    }

    int lua_getMethodInfo(lua_State* L)
    {
        IL2CPP::MethodInfo* methodInfo = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
            const char* methodName = luaL_checkstring(L, 2);
            int argc = luaL_checkinteger(L, 3);

            methodInfo = Reflections::FindMethod(klass, methodName, argc);
        }
        else
        {
            const char* assembly = luaL_checkstring(L, 1);
            const char* namezpace = luaL_checkstring(L, 2);
            const char* klass = luaL_checkstring(L, 3);
            const char* method = luaL_checkstring(L, 4);
            int argc = luaL_checkinteger(L, 5);

            methodInfo = IL2CPP::Resolver::FindMethod(assembly, namezpace, klass, method, argc);
        }

        if (methodInfo)
        {
            lua_pushlightuserdata(L, methodInfo);
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

        IL2CPP::Il2CppClass* klass = IL2CPP::Resolver::FindClass(assembly, namezpace, klassName);

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

    int lua_getStaticField(lua_State* L)
    {
        IL2CPP::Il2CppClass* klass = nullptr;
        const char* fieldName = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
            fieldName = luaL_checkstring(L, 2);
        }
        else
        {
            luaL_error(L, "Use picka.getFieldStatic(klass, fieldName)!");
        }

        if (!klass) return 0;

        void* fieldInfo = Reflections::FindField(klass, fieldName, true);
        if (!fieldInfo)
        {
            // LOGI("Field not found! %s", fieldName);
            lua_pushnil(L);
            return 1;
        }

        uintptr_t value = Reflections::GetFieldValue(nullptr, fieldInfo, true);
        LuaBridge::Helper::luaPushUintptr(L, value);

        return 1;
    }

    int lua_setStaticField(lua_State* L)
    {
        if (!lua_islightuserdata(L, 1)) return luaL_error(L, "Arg 1 must be class pointer");
        IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
        const char* fieldName = luaL_checkstring(L, 2);

        void* fieldInfo = Reflections::FindField(klass, fieldName, true);
        if (!fieldInfo) return luaL_error(L, "Field not found!");

        uintptr_t value = LuaBridge::Helper::luaToUintptr(L, 3);
        Reflections::SetFieldValue(nullptr, fieldInfo, value, true);

        return 0;
    }

    int lua_getField(lua_State* L)
    {
        void* instance = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            instance = lua_touserdata(L, 1);
        }
        else if (lua_isnumber(L, 1))
        {
            instance = (void*)(uintptr_t)lua_tointeger(L, 1);
        }

        const char* fieldName = luaL_checkstring(L, 2);

        if (!instance) 
        {
            M_LOGE("Cannot allocate instance!");
            return 0;
        }

        IL2CPP::Il2CppClass* klass = IL2CPP::object_get_class(instance);
        void* fieldInfo = Reflections::FindField(klass, fieldName, false);

        if (!fieldInfo)
        {
            // LOGI("--- FIELD NOT FOUND: %s ---", fieldName);
            lua_pushnil(L);
            return 1;
        }

        uintptr_t value = Reflections::GetFieldValue(instance, fieldInfo, false);
        LuaBridge::Helper::luaPushUintptr(L, value);

        return 1;
    }

    int lua_setField(lua_State* L)
    {
        void* instance = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            instance = lua_touserdata(L, 1);
        }
        else if (lua_isnumber(L, 1))
        {
            instance = (void*)(uintptr_t)lua_tointeger(L, 1);
        }

        const char* fieldName = luaL_checkstring(L, 2);

        if (!instance) 
        {
            M_LOGE("Cannot allocate instance!");
            return 0;
        }

        IL2CPP::Il2CppClass* klass = IL2CPP::object_get_class(instance);
        void* fieldInfo = Reflections::FindField(klass, fieldName, false);

        if (!fieldInfo)
        {
            M_LOGE("Cannot find field! %s", fieldName);
            lua_pushnil(L);
            return 1;
        }

        uintptr_t value = LuaBridge::Helper::luaToUintptr(L, 3);
        Reflections::SetFieldValue(instance, fieldInfo, value, false);

        return 0;
    }

    int lua_getFieldOffset(lua_State* L)
    {
        IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
        const char* fieldName = luaL_checkstring(L, 2);

        if (!klass) return 0;

        size_t offset = IL2CPP::Resolver::GetFieldOffset(klass, fieldName);
        
        lua_pushinteger(L, (lua_Integer)offset);
        return 1;
    }

    void RegisterReflection(lua_State* L)
    {
        lua_pushcfunction(L, lua_getMethodAddr);
        lua_setfield(L, -2, "getMethodAddr");
        lua_pushcfunction(L, lua_getMethodInfo);
        lua_setfield(L, -2, "getMethodInfo");
        lua_pushcfunction(L, lua_getClass);
        lua_setfield(L, -2, "getClass");
        lua_pushcfunction(L, lua_getStaticField);
        lua_setfield(L, -2, "getFieldStatic");
        lua_pushcfunction(L, lua_setStaticField);
        lua_setfield(L, -2, "setFieldStatic");
        lua_pushcfunction(L, lua_getField);
        lua_setfield(L, -2, "getField");
        lua_pushcfunction(L, lua_setField);
        lua_setfield(L, -2, "setField");
        lua_pushcfunction(L, lua_getFieldOffset);
        lua_setfield(L, -2, "getFieldOffset");
    }
}