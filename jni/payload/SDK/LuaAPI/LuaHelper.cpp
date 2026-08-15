#include "LuaHelper.h"
#include "../Il2Cpp/Il2CppAPI.h"
#include <cstring>

namespace LuaBridge::Helper
{
    void setTypedValue(lua_State* L,  int luaValueIdx, void* addr, const IL2CPP::Il2CppType* fieldType)
    {
        uint8_t type_enum = IL2CPP::type_get_type(fieldType);

        switch (type_enum)
        {
        case 0x02:
            *(bool*)addr = lua_toboolean(L, luaValueIdx);
            return;

        case 0x0C:
            *(float*)addr = lua_tonumber(L, luaValueIdx);
            return;

        case 0x03: case 0x08:
            *(int32_t*)addr = lua_tointeger(L, luaValueIdx);
            return;

        case 0x04: 
            *(int8_t*)addr = (int8_t)lua_tointeger(L, luaValueIdx);
            return;
        
        case 0x05:
            *(uint8_t*)addr = (uint8_t)lua_tointeger(L, luaValueIdx);
            return;
        
        case 0x06:
            *(int16_t*)addr = (int16_t)lua_tointeger(L, luaValueIdx);
            return;

        case 0x07:
            *(uint16_t*)addr = (uint16_t)lua_tointeger(L, luaValueIdx);
            return;

        case 0x0D:
            *(double*)addr = lua_tonumber(L, luaValueIdx);
            return;

        case 0x09:
            *(uint32_t*)addr = (uint32_t)lua_tointeger(L, luaValueIdx);
            return;

        case 0x0E:
        {
            if (lua_type(L, luaValueIdx) == LUA_TSTRING)
            {
                const char* str = lua_tostring(L, luaValueIdx);
                void* il2cppStr = IL2CPP::new_string(str);
                *(void**)addr = il2cppStr;
            }
            else
            {
                *(uintptr_t*)addr = luaToUintptr(L, luaValueIdx);
            }

            return;
        }
        
        default:
            *(uintptr_t*)addr = luaToUintptr(L, luaValueIdx);
            return;
        }
    }

    void fillStructFromTable(lua_State* L, int tableIdx, IL2CPP::Il2CppClass* klass, void* buffer)
    {
        memset(buffer, 0, IL2CPP::class_value_size(klass, nullptr));

        void* iter = 0;

        while (void* field = IL2CPP::class_get_fields(klass, &iter))
        {
            const char* fName = IL2CPP::field_get_name(field);
            lua_getfield(L, tableIdx, fName);

            if (!lua_isnil(L, -1))
            {
                size_t offset = IL2CPP::field_get_offset(field);

                if (offset >= sizeof(IL2CPP::Il2CppObject)) 
                {
                    offset -= sizeof(IL2CPP::Il2CppObject);
                }

                const IL2CPP::Il2CppType* fType = IL2CPP::field_get_type(field);
                setTypedValue(L, -1, (char*)buffer + offset, fType);
            }

            lua_pop(L, 1);
        }
    }

    uintptr_t luaToUintptr(lua_State* L, int idx)
    {
        if (lua_isboolean(L, idx)) return (uintptr_t)lua_toboolean(L, idx);
        if (lua_isinteger(L, idx)) return (uintptr_t)lua_tointeger(L, idx);
        if (lua_islightuserdata(L, idx)) return (uintptr_t)lua_touserdata(L, idx);
        if (lua_isnumber(L, idx)) return (uintptr_t)lua_tonumber(L, idx);

        return 0;
    }

    void luaPushUintptr(lua_State* L, uintptr_t value)
    {
        if (value > 0xFFFFFFFF)
            lua_pushlightuserdata(L, (void*)value);
        else
            lua_pushinteger(L, (lua_Integer)value);
    }
} 
