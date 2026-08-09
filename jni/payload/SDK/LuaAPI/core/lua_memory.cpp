#include "lua_memory.h"

#include "../../../log.h"
#include "../../Il2Cpp/Il2CppResolver.h"

namespace API
{
    int lua_getArrayLength(lua_State* L)
    {
        void* array = lua_touserdata(L, 1);
        if (!array)
        {
            M_LOGE("Argument should be array!");
            lua_pushnil(L);
            return 0;
        }

        lua_pushinteger(L, (int)IL2CPP::array_length(array));
        return 1;
    }

    int lua_getArrayElement(lua_State* L)
    {
        void* array = lua_touserdata(L, 1);
        int index = luaL_checkinteger(L, 2);

        if (!array)
        {
            M_LOGE("Argument should be array!");
            lua_pushnil(L);
            return 0;
        }

        uint32_t len = IL2CPP::array_length(array);
        if (index < 0 || index > (int)len)
        {
            M_LOGE("Index out of array's length!");
            lua_pushnil(L);
            return 0;
        }

        uintptr_t header = (uintptr_t)IL2CPP::array_object_header_size();
        uintptr_t elementAddr = (uintptr_t)array + header + (index * sizeof(void*));

        uintptr_t value = *(uintptr_t*)elementAddr;

        if (value)
        {
            lua_pushlightuserdata(L, (void*)value);
        }
        else
        {
            lua_pushnil(L);
        }

        return 1;
    }

    int lua_readFloat(lua_State* L)
    {
        uintptr_t addr;

        if (lua_isnumber(L, 1)) 
        {
            addr = (uintptr_t)luaL_checkinteger(L, 1);
        } 
        else 
        {
            addr = (uintptr_t)lua_touserdata(L, 1);
        }

        int offset = luaL_optinteger(L, 2, 0);

        if (addr == 0) return 0;

        float value = *(float*)(addr + offset);
        lua_pushnumber(L, value);
        return 1;
    }

    void RegisterMemory(lua_State* L)
    {
        lua_pushcfunction(L, lua_getArrayLength);
        lua_setfield(L, -2, "getArrayLength");
        lua_pushcfunction(L, lua_getArrayElement);
        lua_setfield(L, -2, "getArrayElement");

        lua_pushcfunction(L, lua_readFloat);
        lua_setfield(L, -2, "readFloat");
    }
}