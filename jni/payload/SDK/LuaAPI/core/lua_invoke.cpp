#include "lua_invoke.h"

#include "libs/libffi/include/ffi.h"
#include "../../../log.h"
#include "../../Il2Cpp/Il2CppResolver.h"

#include "../LuaHelper.h"
#include "lua_wrap.h"
#include <unordered_map>

static ffi_type* get_ffi_type(uint8_t il2cpp_type_enum) 
{
    // LOGI("Mapping IL2CPP type 0x%02X", il2cpp_type_enum);

    switch (il2cpp_type_enum) 
    {
        case 0x01: return &ffi_type_void;    // void
        case 0x02: return &ffi_type_uint8;   // bool
        case 0x03: return &ffi_type_sint32;  // int
        case 0x04: return &ffi_type_sint8;
        case 0x05: return &ffi_type_uint8;
        case 0x06: return &ffi_type_sint16;
        case 0x07: return &ffi_type_uint16;
        case 0x08: return &ffi_type_sint32;
        case 0x0C: return &ffi_type_float;   // float and System.Single
        case 0x0D: return &ffi_type_double;
        case 0x09: return &ffi_type_uint32;
        case 0x0e:                           // string
        case 0x12:                           // class
        case 0X1C:                           // object
        case 0x0f:                           // ptr
            return &ffi_type_pointer;       
        default: 
            return &ffi_type_pointer;
    }
}

static ffi_type* build_struct_ffi_type(IL2CPP::Il2CppClass* klass)
{
    static std::unordered_map<void*, ffi_type*> cache;
    auto found = cache.find(klass);
    if (found != cache.end())
        return found->second;
        
    std::vector<ffi_type*> elements;

    void* iter = nullptr;
    void* field;
    while ((field = IL2CPP::class_get_fields(klass, &iter)) != nullptr)
    {
        if (IL2CPP::field_get_flags(field) & 0x0010)
            continue;

        const IL2CPP::Il2CppType* fieldType = IL2CPP::field_get_type(field);
        uint8_t rawType = IL2CPP::type_get_type(fieldType);
        elements.push_back(get_ffi_type(rawType));
    }
    elements.push_back(nullptr); 

    ffi_type* structType = new ffi_type();
    structType->size = 0;
    structType->alignment = 0;
    structType->type = FFI_TYPE_STRUCT;

    ffi_type** elems = new ffi_type*[elements.size()];
    std::copy(elements.begin(), elements.end(), elems);
    structType->elements = elems;

    cache[klass] = structType;
    return structType;
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
                
                const IL2CPP::Il2CppType* typeStruct = IL2CPP::method_get_param(methodInfo, param_idx);
                
                if (lua_istable(L, lua_idx)) 
                {
                    IL2CPP::Il2CppClass* structClass = IL2CPP::class_from_type(typeStruct);
                    uint32_t align = 0;
                    int size = IL2CPP::class_value_size(structClass, &align);
                    
                    void* structBuffer = alloca(size);
                    LuaBridge::Helper::fillStructFromTable(L, lua_idx, structClass, structBuffer);

                    arg_types[i] = build_struct_ffi_type(structClass);
                    arg_values[i] = structBuffer;
                    continue; 
                }

                uint8_t raw_type = IL2CPP::type_get_type(typeStruct);

                IL2CPP::Il2CppClass* maybeStructClass = (raw_type == 0x11) ? IL2CPP::class_from_type(typeStruct) : nullptr;
                bool isEnum = maybeStructClass && IL2CPP::class_is_enum(maybeStructClass);

                if (raw_type == 0x11 &&  !isEnum)
                {
                    IL2CPP::Il2CppClass* structClass = IL2CPP::class_from_type(typeStruct);

                    uint32_t align = 0;
                    int size = IL2CPP::class_value_size(structClass, &align);

                    void* structBuffer = alloca(size);
                    memset(structBuffer, 0, size);

                    if (void* cw = luaL_testudata(L, lua_idx, API::CLASS_WRAPPER_META))
                    {
                        API::ClassWrapper* w = (API::ClassWrapper*)cw;
                        if (w->instance)
                        {
                            memcpy(structBuffer, (char*)w->instance + sizeof(IL2CPP::Il2CppObject), size);

                            if (size == 8)
                            {
                                float x, y;
                                memcpy(&x, structBuffer, 4);
                                memcpy(&y, (char*)structBuffer + 4, 4);
                                // M_LOGI("Vector2 copied from instance: X=%.2f Y=%.2f (raw instance=%p, header size=%zu)",
                                //     x, y, w->instance, sizeof(IL2CPP::Il2CppObject));
                            }
                        }
                    }
                    else if (void* sw = luaL_testudata(L, lua_idx, API::STRUCT_WRAPPER_META))
                    {
                        API::StructWrapper* s = (API::StructWrapper*)sw;
                        if (s->base)
                            memcpy(structBuffer, s->base, size);
                    }
                    else if (lua_islightuserdata(L, lua_idx))
                    {
                        void* raw = lua_touserdata(L, lua_idx);
                        if (raw)
                            memcpy(structBuffer, (char*)raw + sizeof(IL2CPP::Il2CppObject), size);
                    }

                    arg_types[i] = build_struct_ffi_type(structClass);
                    arg_values[i] = structBuffer;
                    continue;
                }

                if (raw_type == 0x11 && isEnum)
                {
                    storage[i].i = (int32_t)lua_tointeger(L, lua_idx);
                    arg_types[i] = &ffi_type_sint32;
                    arg_values[i] = &storage[i];
                    continue;
                }

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