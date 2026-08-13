#include "lua_invoke.h"

#include "libs/libffi/include/ffi.h"
#include "../../../log.h"
#include "../../Il2Cpp/Il2CppResolver.h"

#include "../LuaHelper.h"

static ffi_type* get_ffi_type(uint8_t il2cpp_type_enum) 
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

namespace Invoke
{
    uintptr_t CallMethod(lua_State* L, IL2CPP::MethodInfo* methodInfo, int argsBase)
    {
        if (!methodInfo) return 0; 

        bool is_static = (methodInfo->flags & 0x0010);
        int lua_args_count = lua_gettop(L) - argsBase + 1;

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
            int lua_idx = argsBase + i; 
            
            if (!is_static && i == 0) 
            {
                storage[i].p = (uintptr_t)lua_touserdata(L, lua_idx);
                arg_types[i] = &ffi_type_pointer;
            } 
            else 
            {
                int param_idx = is_static ? i : i - 1;
                // auto* param = methodInfo->parameters[param_idx];
                
                const IL2CPP::Il2CppType* typeStruct = IL2CPP::method_get_param(methodInfo, param_idx);
                /* uintptr_t addr = (uintptr_t)typeStruct;
                LOGI("Param %d hex dump: %02X %02X %02X %02X %02X %02X %02X %02X | %02X %02X %02X %02X", 
                    param_idx,
                    *(uint8_t*)(addr), *(uint8_t*)(addr+1), *(uint8_t*)(addr+2), *(uint8_t*)(addr+3),
                    *(uint8_t*)(addr+4), *(uint8_t*)(addr+5), *(uint8_t*)(addr+6), *(uint8_t*)(addr+7),
                    *(uint8_t*)(addr+8), *(uint8_t*)(addr+9), *(uint8_t*)(addr+10), *(uint8_t*)(addr+11)); 
                    
                    LOGI("Method: %s, ParamIdx: %d, TypePtr: %p", methodInfo->name, param_idx, typeStruct);
                */
                
                if (lua_istable(L, lua_idx)) 
                {
                    IL2CPP::Il2CppClass* structClass = IL2CPP::class_from_type(typeStruct);
                    uint32_t align = 0;
                    int size = IL2CPP::class_value_size(structClass, &align);
                    
                    void* structBuffer = alloca(size);
                    LuaBridge::Helper::fillStructFromTable(L, lua_idx, structClass, structBuffer);

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

                if (raw_type == 0x0E && lua_type(L, lua_idx) == LUA_TSTRING)
                {
                    const char* str = lua_tostring(L, lua_idx);
                    void* il2cppString = IL2CPP::new_string(str);

                    storage[i].p = (uintptr_t)il2cppString;
                    arg_values[i] = &storage[i];
                    continue;
                }

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

        return result;
    }

    uintptr_t CallMethodInternal(lua_State* L, IL2CPP::MethodInfo* methodInfo, std::function<void(lua_State*)> pushArgs)
    {
        int savedTop = lua_gettop(L);

        int argsBase = savedTop + 1;
        pushArgs(L);

        uintptr_t result = CallMethod(L, methodInfo, argsBase);

        lua_settop(L, savedTop);
        return result;
    }
}

namespace API
{
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

        uintptr_t result = Invoke::CallMethod(L, methodInfo, 2);

        lua_pushinteger(L, (lua_Integer)result);

        return 1;
    }

    void RegisterInvoker(lua_State* L)
    {
        lua_pushcfunction(L, lua_callNative);
        lua_setfield(L, -2, "callNative");
        lua_pushcfunction(L, lua_callMethod);
        lua_setfield(L, -2, "callMethod");
    }
}