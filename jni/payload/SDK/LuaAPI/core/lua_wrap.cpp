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
    static const char* STRUCT_WRAPPER_META = "picka.StructWrapper";
    static const char* ARRAY_WRAPPER_META = "picka.ArrayWrapper";

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

        int nargc = lua_gettop(L);

        int callbackIdx;
        int overloadArgc;

        if (nargc == 2)
        {
            if (!lua_isfunction(L, 2)) return luaL_error(L, "Method hook() expected a function!");

            callbackIdx = 2;
            overloadArgc = -1;
        }
        else if (nargc == 3)
        {
            if (!lua_isnumber(L, 2)) return luaL_error(L, "Expected overload argument count as 2nd argument");
            if (!lua_isfunction(L, 3)) return luaL_error(L, "Method hook() expected a function as last argument!");

            overloadArgc = (int)luaL_checkinteger(L, 2);
            callbackIdx = 3;
        }
        else
        {
            return luaL_error(L, "hook() expects hook(function) or hook(argc, function)!");
        }

        IL2CPP::MethodInfo* method = cacheMethodInfo(h->klass, h->name.c_str(), overloadArgc);

        if (!method)
            return luaL_error(L, "Cannot resolve method '%s' for hook", h->name.c_str());

        uintptr_t addr = IL2CPP::Resolver::GetMethodPtr(method);
        if (!addr)
            return luaL_error(L, "Cannot get method pointer for '%s'", h->name.c_str());

        return Hook::RegisterHook(L, addr, -1, callbackIdx);
    }

    int methodHandle_index(lua_State* L)
    {
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "hook") == 0)
        {
            lua_pushcfunction(L, methodHandle_hook);
            return 1;
        }

        lua_pushnil(L);
        return 1;
    }

    struct ArrayWrapper
    {
        void* arrPtr;
        IL2CPP::Il2CppClass* elementClass;
        bool elementAreValueType; // for arrays of pointers, like Main.player, Main.npc
        size_t elementSize;
    };

    void PushArrayWrapper(lua_State* L, void* arrPtr)
    {
        if (!arrPtr)
        {   
            lua_pushnil(L);
            return;
        }

        IL2CPP::Il2CppClass* arrayKlass = IL2CPP::object_get_class(arrPtr);
        IL2CPP::Il2CppClass* elementClass = arrayKlass->_1.element_class;
        uint32_t elementSize = arrayKlass->_2.element_size;

        ArrayWrapper* a = (ArrayWrapper*)lua_newuserdata(L, sizeof(ArrayWrapper));
        a->arrPtr = arrPtr;
        a->elementClass = elementClass;
        a->elementAreValueType = elementClass ? IL2CPP::class_is_valuetype(elementClass) : false;
        a->elementSize = elementSize;

        luaL_getmetatable(L, ARRAY_WRAPPER_META);
        lua_setmetatable(L, -2);
    }

    struct StructWrapper
    {
        void* base;
        IL2CPP::Il2CppClass* klass;
    };

    void PushStructWrapper(lua_State* L, void* base, IL2CPP::Il2CppClass* klass)
    {
        StructWrapper* s = (StructWrapper*)lua_newuserdata(L, sizeof(StructWrapper));
        s->base = base;
        s->klass = klass;

        luaL_getmetatable(L, STRUCT_WRAPPER_META);
        lua_setmetatable(L, -2);    
    }

    static int arrayWrapper_index(lua_State* L)
    {
        ArrayWrapper* a = (ArrayWrapper*)luaL_checkudata(L, 1, ARRAY_WRAPPER_META);
        int index = (int)luaL_checkinteger(L, 2);

        uint32_t len = IL2CPP::array_length(a->arrPtr);
        if (index < 0 || index >= (int)len)
            return luaL_error(L, "Index '%d' is out of bounds! (array length: %u)", index, len);

        uintptr_t header = (uintptr_t)IL2CPP::array_object_header_size();
        uintptr_t elementAddr = (uintptr_t)a->arrPtr + header + (index * a->elementSize);

        if (a->elementAreValueType)
        {
            API::PushStructWrapper(L, (void*)elementAddr, a->elementClass);
            return 1;
        }

        uintptr_t objPtr = *(uintptr_t*)elementAddr;
        if (objPtr)
            LuaBridge::Helper::luaPushUintptr(L, objPtr);
        else
            lua_pushnil(L);

        return 1;
    }
    static int arrayWrapper_newindex(lua_State* L)
    {
        ArrayWrapper* a = (ArrayWrapper*)luaL_checkudata(L, 1, ARRAY_WRAPPER_META);
        int index = (int)luaL_checkinteger(L, 2);

        uint32_t len = IL2CPP::array_length(a->arrPtr);
        if (index < 0 || index >= (int)len)
            return luaL_error(L, "Index '%d' is out of bounds! (array length: %u)", index, len);

        uintptr_t header = (uintptr_t)IL2CPP::array_object_header_size();
        uintptr_t elementAddr = (uintptr_t)a->arrPtr + header + (index * a->elementSize);

        if (a->elementAreValueType)
        {
            return luaL_error(L, "Direct assignment to value-type array elements not supported. Modify element of array instead!");
        }

        uintptr_t value  = LuaBridge::Helper::luaToUintptr(L, 3);
        *(uintptr_t*)elementAddr = value;
        return 0;
    }
    static int arrayWrapper_len(lua_State* L)
    {
        ArrayWrapper* a = (ArrayWrapper*)luaL_checkudata(L, 1, ARRAY_WRAPPER_META);
        lua_pushinteger(L, (lua_Integer)IL2CPP::array_length(a->arrPtr));
        return 1;
    }


    int structWrapper_index(lua_State* L)
    {
        StructWrapper* s = (StructWrapper*)luaL_checkudata(L, 1, STRUCT_WRAPPER_META);
        const char* key = luaL_checkstring(L, 2);
        
        void* fieldInfo = cacheFieldInfo(s->klass, key, false);
        if (!fieldInfo) return luaL_error(L, "Struct field '%s' not found", key);

        size_t offset = IL2CPP::field_get_offset(fieldInfo);
        void* addr = (char*)s->base + offset;
        
        const IL2CPP::Il2CppType* fieldType = IL2CPP::field_get_type(fieldInfo);
        IL2CPP::Il2CppClass* fieldClass = IL2CPP::class_from_type(fieldType);

        Reflections::PushTypedValue(L, addr, fieldType, fieldClass);
        return 1;
    }
    int structWrapper_newindex(lua_State* L)
    {
        StructWrapper* s = (StructWrapper*)luaL_checkudata(L, 1, STRUCT_WRAPPER_META);
        const char* key = luaL_checkstring(L, 2);
        
        void* fieldInfo = cacheFieldInfo(s->klass, key, false);
        if (!fieldInfo) return luaL_error(L, "Struct field '%s' not found", key);

        size_t offset = IL2CPP::field_get_offset(fieldInfo);
        void* addr = (char*)s->base + offset;
        
        const IL2CPP::Il2CppType* fieldType = IL2CPP::field_get_type(fieldInfo);

        LuaBridge::Helper::setTypedValue(L, 3, addr, fieldType);
        return 0;
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

    int classWrapper_ctorCall(lua_State* L)
    {
        IL2CPP::Il2CppClass* klass = (IL2CPP::Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));

        int argc = lua_gettop(L);

        IL2CPP::MethodInfo* ctor = cacheMethodInfo(klass, ".ctor", argc);
        if (!ctor)
            return luaL_error(L, "Constructor with %d args not found for this class", argc);

        IL2CPP::Il2CppObject* instance = IL2CPP::object_new(klass);
        if (!instance)
            return luaL_error(L, "il2cpp_object_new returned null - failed to allocate instance");
    
        lua_pushlightuserdata(L, instance);
        lua_insert(L, 1);

        Invoke::CallMethod(L, ctor, 1);

        ClassWrapper* w = (ClassWrapper*)lua_newuserdata(L, sizeof(ClassWrapper));
        w->klass = klass;
        w->instance = instance;

        luaL_getmetatable(L, CLASS_WRAPPER_META);
        lua_setmetatable(L, -2);

        return 1;
    }

    int classWrapper_index(lua_State* L)
    {
        ClassWrapper* w = (ClassWrapper*)luaL_checkudata(L, 1, CLASS_WRAPPER_META);
        const char* fieldName = luaL_checkstring(L, 2);

        bool isStatic = (w->instance == nullptr);

        if (isStatic && strcmp(fieldName, "new") == 0)
        {
            lua_pushlightuserdata(L, w->klass);
            lua_pushcclosure(L, classWrapper_ctorCall, 1);
            return 1;
        }

        IL2CPP::Il2CppClass* klass = w->instance ? IL2CPP::object_get_class(w->instance) : w->klass;

        void* fieldInfo = cacheFieldInfo(klass, fieldName, isStatic);
        if (fieldInfo)
        {
            const IL2CPP::Il2CppType* fieldType = IL2CPP::field_get_type(fieldInfo);
            IL2CPP::Il2CppClass* fieldClass = IL2CPP::class_from_type(fieldType);
            size_t offset = IL2CPP::field_get_offset(fieldInfo);
 
            void* addr;
            if (isStatic)
            {
                void* staticData = IL2CPP::class_get_static_field_data(klass);
                addr = (char*)staticData + offset;
            }
            else
            {
                addr = (char*)w->instance + offset;
            }

            uint8_t type_enum = IL2CPP::type_get_type(fieldType);

            if (type_enum == 0x1D)
            {
                uintptr_t arrayPtr = *(uintptr_t*)addr;
                PushArrayWrapper(L, (void*)arrayPtr);
                return 1;
            }

            Reflections::PushTypedValue(L, addr, fieldType, fieldClass);
            return 1;
        }

        void* mem = lua_newuserdata(L, sizeof(MethodHandle));
        MethodHandle* h = new (mem) MethodHandle();
        h->klass = w->klass;
        h->instance = w->instance;
        h->name = fieldName;

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

        const IL2CPP::Il2CppType* fieldType = IL2CPP::field_get_type(fieldInfo);
        IL2CPP::Il2CppClass* fieldClass = IL2CPP::class_from_type(fieldType);
        size_t offset = IL2CPP::field_get_offset(fieldInfo);

        void* addr;
        if (isStatic)
        {
            void* staticData = IL2CPP::class_get_static_field_data(klass);
            addr = (char*)staticData + offset;
        }
        else
        {
            addr = (char*)w->instance + offset;
        }
            
        LuaBridge::Helper::setTypedValue(L, 3, addr, fieldType);
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

        luaL_newmetatable(L, ARRAY_WRAPPER_META);
        lua_pushcfunction(L, arrayWrapper_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, arrayWrapper_newindex);
        lua_setfield(L, -2, "__newindex");
        lua_pushcfunction(L, arrayWrapper_len);
        lua_setfield(L, -2, "__len");
        lua_pop(L, 1);

        luaL_newmetatable(L, STRUCT_WRAPPER_META);
        lua_pushcfunction(L, structWrapper_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, structWrapper_newindex);
        lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);

        lua_pushcfunction(L, lua_pickaClass);
        lua_setfield(L, -2, "class");
        lua_pushcfunction(L, lua_pickaWrap);
        lua_setfield(L, -2, "wrap");
    }
}