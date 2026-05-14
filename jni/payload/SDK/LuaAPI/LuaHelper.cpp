#include "LuaHelper.h"
#include "../Il2Cpp/Il2CppAPI.h"
#include <cstring>

namespace LuaBridge::Helper
{
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
                uint8_t type_enum = IL2CPP::type_get_type(fType);

                if (type_enum == 0x02) 
                {
                    bool val = lua_toboolean(L, -1);
                    *(bool*)((char*)buffer + offset) = val;
                }
                else if (type_enum == 0x0C) 
                {
                    float val = (float)lua_tonumber(L, -1);
                    *(float*)((char*)buffer + offset) = val;
                }
                else if (type_enum == 0x03 || type_enum == 0x08 || type_enum == 0x05) 
                { 
                    int32_t val = (int32_t)lua_tointeger(L, -1);
                    if (type_enum == 0x05) *(uint8_t*)((char*)buffer + offset) = (uint8_t)val; // BYTE
                    else *(int32_t*)((char*)buffer + offset) = val;
                }
                else 
                {
                    uintptr_t val = (uintptr_t)lua_tonumber(L, -1);
                    *(uintptr_t*)((char*)buffer + offset) = val;
                }
            }

            lua_pop(L, 1);
        }
    }
} 
