#include "LuaBridge.h"
#include "LuaHelper.h"
#include "../Il2Cpp/Il2CppAPI.h"
#include "../Il2Cpp/Il2CppResolver.h"
#include "../hook.h"
#include "log.h"
#include <android/log.h>
#include <string>
#include <map>
#include "libs/libffi/include/ffi.h"

#define MAX_HOOKS 256

ffi_type* get_ffi_type(uint8_t il2cpp_type_enum) 
{
    // LOGI("Mapping IL2CPP type 0x%02X", il2cpp_type_enum);

    switch (il2cpp_type_enum) 
    {
        case 0x01: return &ffi_type_void;    // void
        case 0x02: return &ffi_type_uint8;   // bool
        case 0x03: return &ffi_type_sint32;  // int
        case 0x08: return &ffi_type_sint32;
        case 0x0C: return &ffi_type_float;   // float and System.Single
        case 0x09: return &ffi_type_double;  // double
        case 0x0e:                           // string
        case 0x12:                           // class
        case 0X1C:                           // object
        case 0x0f:                           // ptr
            return &ffi_type_pointer;       
        default: 
            return &ffi_type_pointer;
    }
}

namespace LuaBridge
{
    lua_State* g_Lstate = nullptr;

    static std::map<uintptr_t, int> addrToSlot;

    template<std::size_t... Is>
    constexpr auto make_proxy_addresses(std::index_sequence<Is...>) 
    {
        return std::array<uintptr_t, sizeof...(Is)>{ (uintptr_t)HookHandler<Is>... };
    }

    static auto proxy_array = make_proxy_addresses(std::make_index_sequence<MAX_HOOKS>{}); // make 256 free hooks at once
    uintptr_t* proxy_addresses = proxy_array.data();

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

    int lua_getMethodAddr(lua_State* L)
    {
        IL2CPP::MethodInfo* methodInfo = nullptr;

        if (lua_islightuserdata(L, 1))
        {
            // if we want write picka.getMethodPtr(klass, methodName, argsCount)
            IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
            const char* methodName = luaL_checkstring(L, 2);
            int args = luaL_checkinteger(L, 3);

            methodInfo = IL2CPP::class_get_method_from_name(klass, methodName, args);
        }
        else
        {
            const char* assembly = luaL_checkstring(L, 1);
            const char* namezpace = luaL_checkstring(L, 2);
            const char* klass = luaL_checkstring(L, 3);
            const char* method = luaL_checkstring(L, 4);
            int args = luaL_checkinteger(L, 5);

            methodInfo = IL2CPP::Resolver::FindMethod(assembly, namezpace, klass, method, args);
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
            int args = luaL_checkinteger(L, 3);

            methodInfo = IL2CPP::class_get_method_from_name(klass, methodName, args);
        }
        else
        {
            const char* assembly = luaL_checkstring(L, 1);
            const char* namezpace = luaL_checkstring(L, 2);
            const char* klass = luaL_checkstring(L, 3);
            const char* method = luaL_checkstring(L, 4);
            int args = luaL_checkinteger(L, 5);

            methodInfo = IL2CPP::Resolver::FindMethod(assembly, namezpace, klass, method, args);
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

        void* fieldInfo = IL2CPP::class_get_field_from_name(klass, fieldName);
        if (!fieldInfo)
        {
            // LOGI("Field not found! %s", fieldName);
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
        IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
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
        void* fieldInfo = IL2CPP::Resolver::FindField(klass, fieldName);

        if (!fieldInfo)
        {
            // LOGI("--- FIELD NOT FOUND: %s ---", fieldName);
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
        void* fieldInfo = IL2CPP::class_get_field_from_name(klass, fieldName);

        if (!fieldInfo)
        {
            M_LOGE("Cannot find field! %s", fieldName);
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

    int lua_getFieldOffset(lua_State* L)
    {
        IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, 1);
        const char* fieldName = luaL_checkstring(L, 2);

        if (!klass) return 0;

        size_t offset = IL2CPP::Resolver::GetFieldOffset(klass, fieldName);
        
        lua_pushinteger(L, (lua_Integer)offset);
        return 1;
    }

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

        uintptr_t args[16] = {0}; 

        for (int i = 2; i <= n && (i - 2) < 16; ++i)
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
            else if (lua_isnumber(L, i)) 
            {
                // args[i - 2] = (uintptr_t)lua_tonumber(L, i);
                // double d = lua_tonumber(L, i);
                // memcpy(&args[i - 2], &d, sizeof(double));

                float f = (float)lua_tonumber(L, i);
                uint32_t bits;
                memcpy(&bits, &f, sizeof(float));
                args[i - 2] = (uintptr_t)bits;
            }
        }

        typedef uintptr_t (*GenericFn)(
            uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t,
            uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t
        );

        uintptr_t result = ((GenericFn)addr)(
            args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7],
            args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15]
        );

        lua_pushinteger(L, (lua_Integer)result);
        return 1;
    }

    int lua_callMethod(lua_State* L)
    {
        IL2CPP::MethodInfo* methodInfo = (IL2CPP::MethodInfo*)lua_touserdata(L, 1);
        if (!methodInfo) return 0; 

        bool is_static = (methodInfo->flags & 0x0010);
        int lua_args_count = lua_gettop(L) - 1;

        ffi_cif cif;
        ffi_type* arg_types[32];
        void* arg_values[32];

        union ArgStorage
        {
            uintptr_t p;
            float f;
            int32_t i;
        } storage[32];

        for (int i = 0; i < lua_args_count; i++) 
        {
            int lua_idx = i + 2; 
            
            if (!is_static && i == 0) 
            {
                storage[i].p = (uintptr_t)lua_touserdata(L, lua_idx);
                arg_types[i] = &ffi_type_pointer;
            } 
            else 
            {
                int param_idx = is_static ? i : i - 1;
                auto* param = methodInfo->parameters[param_idx];
                
                const IL2CPP::Il2CppType* typeStruct = IL2CPP::method_get_param(methodInfo, param_idx);
                /* uintptr_t addr = (uintptr_t)typeStruct;
                LOGI("Param %d hex dump: %02X %02X %02X %02X %02X %02X %02X %02X | %02X %02X %02X %02X", 
                    param_idx,
                    *(uint8_t*)(addr), *(uint8_t*)(addr+1), *(uint8_t*)(addr+2), *(uint8_t*)(addr+3),
                    *(uint8_t*)(addr+4), *(uint8_t*)(addr+5), *(uint8_t*)(addr+6), *(uint8_t*)(addr+7),
                    *(uint8_t*)(addr+8), *(uint8_t*)(addr+9), *(uint8_t*)(addr+10), *(uint8_t*)(addr+11)); */
                // LOGI("Method: %s, ParamIdx: %d, TypePtr: %p", methodInfo->name, param_idx, typeStruct);
                
                if (lua_istable(L, lua_idx)) 
                {
                    IL2CPP::Il2CppClass* structClass = IL2CPP::class_from_type(typeStruct);
                    uint32_t align = 0;
                    int size = IL2CPP::class_value_size(structClass, &align);
                    
                    void* structBuffer = alloca(size);
                    Helper::fillStructFromTable(L, lua_idx, structClass, structBuffer);

                    if (size <= 8) 
                    {
                        storage[i].p = 0; 
                        memcpy(&storage[i].p, structBuffer, size);
                        
                        arg_types[i] = &ffi_type_uint64;
                        arg_values[i] = &storage[i].p;
                    } 
                    else 
                    {
                        arg_values[i] = structBuffer;
                        arg_types[i] = &ffi_type_pointer;
                    }
                    continue; 
                }

                uint8_t raw_type = IL2CPP::type_get_type(typeStruct);

                ffi_type* f_type = get_ffi_type(raw_type);
                arg_types[i] = f_type;


                if (f_type == &ffi_type_float) 
                {
                    storage[i].f = (float)lua_tonumber(L, lua_idx);
                } 
                else if (f_type == &ffi_type_sint32 || f_type == &ffi_type_uint8) // Добавь проверку на байт
                {
                    storage[i].i = (int32_t)lua_tointeger(L, lua_idx);
                } 
                else if (lua_isnumber(L, lua_idx))
                {
                    storage[i].p = (uintptr_t)lua_tointeger(L, lua_idx); 
                }
                else if (lua_isnil(L, lua_idx)) 
                {
                    storage[i].p = 0;
                }
                else 
                {
                    storage[i].p = (uintptr_t)lua_touserdata(L, lua_idx);
                }
            }

            arg_values[i] = &storage[i];
        }

        uintptr_t result = 0;

        uint8_t ret_raw_type = (uint8_t)(methodInfo->return_type->bits & 0xFF);
        ffi_type* f_ret_type = get_ffi_type(ret_raw_type);

        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, lua_args_count, f_ret_type, arg_types) == FFI_OK)
            ffi_call(&cif, FFI_FN(methodInfo->methodPointer), &result, arg_values);

        lua_pushinteger(L, (lua_Integer)result);

        return 1;
    }

    int lua_hook(lua_State* L)
    {
        uintptr_t target = (uintptr_t)luaL_checkinteger(L, 1);

        if (addrToSlot.count(target) > 0)
        {
            int slot = addrToSlot[target];
            if (!lua_isfunction(L, 3)) return luaL_error(L, "Arg 3 must be a function");

            lua_pushvalue(L, 3);
            int callback = luaL_ref(L, LUA_REGISTRYINDEX);
            
            registeredHooks[slot].callbacks.push_back(callback);
            M_LOGI("Hook updated: Mod added to chain in slot %d. Total mods: %zu", slot, registeredHooks[slot].callbacks.size());

            lua_pushboolean(L, true);
            return 1;
        }
        else
        {
            int args_count = luaL_checkinteger(L, 2);

            if (current_slot >= MAX_HOOKS)
            {
                lua_pushboolean(L, false);
                return 1;
            }

            if (!lua_isfunction(L, 3)) return luaL_error(L, "Arg 3 must be a function");

            lua_pushvalue(L, 3);
            int r = luaL_ref(L, LUA_REGISTRYINDEX);

            registeredHooks[current_slot].callbacks.push_back(r);
            registeredHooks[current_slot].args_count = args_count;

            int result = hook_function(target, proxy_addresses[current_slot], &registeredHooks[current_slot].original_ptr, 4);

            if (result == 0)
            {
                addrToSlot[target] = current_slot;
                current_slot++;
                lua_pushboolean(L, true);
            }
            else
            {
                lua_pushboolean(L, false);
            }
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

        lua_pushcfunction(L, lua_getArrayLength);
        lua_setfield(L, -2, "getArrayLength");
        lua_pushcfunction(L, lua_getArrayElement);
        lua_setfield(L, -2, "getArrayElement");

        lua_pushcfunction(L, lua_readFloat);
        lua_setfield(L, -2, "readFloat");

        lua_pushcfunction(L, lua_callNative);
        lua_setfield(L, -2, "callNative");
        lua_pushcfunction(L, lua_callMethod);
        lua_setfield(L, -2, "callMethod");

        lua_pushcfunction(L, lua_hook);
        lua_setfield(L, -2, "hook");

        lua_setglobal(L, "picka");
        luaL_dostring(L, "print = picka.log");
    }
}