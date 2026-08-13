#include "lua_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../libs/stb_image/stb_image.h"

#include "../../log.h"
#include "../../Il2Cpp/Il2CppResolver.h"

#include "lua_invoke.h"
#include "lua_reflection.h"

#include <string>

namespace API
{
    static IL2CPP::MethodInfo* FindCompiledSetData(IL2CPP::Il2CppClass* klass)
    {
        void* iter = nullptr;
        IL2CPP::MethodInfo* method;

        while ((method = IL2CPP::class_get_methods(klass, &iter)) != nullptr)
        {
            const char* name = IL2CPP::method_get_name(method);
            if (name && strcmp(name, "SetData") == 0)
            {
                uintptr_t addr = IL2CPP::Resolver::GetMethodPtr(method);
                if (addr) 
                    return (IL2CPP::MethodInfo*)method;
            }
        }

        return nullptr;
    }

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

    int lua_loadTexture(lua_State* L)
    {
        const char* path = luaL_checkstring(L, 1);
        void* graphicsDevice = lua_touserdata(L, 2); // ONLY FOR TEST

        if (lua_islightuserdata(L, 2) || lua_isuserdata(L, 2))
        {
            graphicsDevice = lua_touserdata(L, 2);
        }
        else if (lua_isnumber(L, 2))
        {
            graphicsDevice = (void*)(uintptr_t)lua_tointeger(L, 2);
        }

        int width, height, channels;
        unsigned char* pixels = stbi_load(path, &width, &height, &channels, 4);

        if (!pixels)
        {
            M_LOGE("Failed to load texture: %s (%s)", path, stbi_failure_reason());
            lua_pushnil(L);
            return 1;
        }
        
        int size = width * height;

        IL2CPP::Il2CppClass* texture2DClass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "Microsoft.Xna.Framework.Graphics", "Texture2D");
        if (!texture2DClass) 
        { 
            M_LOGE("Cannot find Texture2D class!"); 
            lua_pushnil(L); 
            return 1; 
        }
        IL2CPP::MethodInfo* ctor = Reflections::FindMethod(texture2DClass, ".ctor", 3);
        if (!ctor) 
        { 
            M_LOGE("Cannot find Texture2D.ctor() method!"); 
            lua_pushnil(L); 
            return 1; 
        }

        IL2CPP::Il2CppClass* colorClass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "Microsoft.Xna.Framework.Graphics", "Color");
        if (!colorClass) 
        { 
            M_LOGE("Cannot find Color class!"); 
            lua_pushnil(L); 
            return 1; 
        }
        IL2CPP::Il2CppClass* objectClass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "System", "Object");
        if (!objectClass) 
        { 
            M_LOGE("Cannot find Object class!"); 
            lua_pushnil(L); 
            return 1; 
        }

        void* textureInstance = IL2CPP::object_new(texture2DClass);
        if (!textureInstance)
        {
            M_LOGE("Cannot create Texture2D instance!");
            lua_pushnil(L);
            return 1;
        }

        Invoke::CallMethodInternal(L, ctor, [&](lua_State* L) {
            lua_pushlightuserdata(L, textureInstance);
            lua_pushlightuserdata(L, graphicsDevice);
            lua_pushinteger(L, width);
            lua_pushinteger(L, height);
        });

        IL2CPP::MethodInfo* setPackedValue = Reflections::FindMethod(colorClass, "set_PackedValue", 1);
        if (!setPackedValue) 
        { 
            M_LOGE("Cannot find Color.set_PackedValue() method!"); 
            lua_pushnil(L); 
            return 1; 
        }

        void* objArray = IL2CPP::array_new(objectClass, size);
        if (!objArray)
        {
            M_LOGE("Cannot create System.Object array!");
            lua_pushnil(L);
            return 1;
        }

        uintptr_t arrayHeader = (uintptr_t)IL2CPP::array_object_header_size();

        for (int i = 0; i < size; ++i)
        {
            uint8_t r = pixels[i * 4 + 0];
            uint8_t g = pixels[i * 4 + 1];
            uint8_t b = pixels[i * 4 + 2];
            uint8_t a = pixels[i * 4 + 3];

            uint32_t packedValue = r | (g << 8) | (b << 16) | (a << 24);

            void* boxedColor = IL2CPP::object_new(colorClass);

            Invoke::CallMethodInternal(L, setPackedValue, [&](lua_State* L) {
                lua_pushlightuserdata(L, boxedColor);
                lua_pushinteger(L, (lua_Integer)packedValue);
            });

            uintptr_t elementAddr = (uintptr_t)objArray + arrayHeader + (i * sizeof(void*));
            *(uintptr_t*)elementAddr = (uintptr_t)boxedColor;
        }

        stbi_image_free(pixels);

        uintptr_t il2cppBase = IL2CPP::GetIl2CppBase();

        IL2CPP::MethodInfo* openSetData = Reflections::FindMethod(texture2DClass, "SetData", 3);
        if (!openSetData) 
        { 
            M_LOGE("Cannot find open generinc SetData<T>() method!"); 
            lua_pushnil(L); 
            return 1; 
        }
        
        void* reflMethod = IL2CPP::method_get_object(openSetData, texture2DClass);
        if (!reflMethod)
        {
            M_LOGE("il2cpp_method_get_object failed for SetData<T>()");
            lua_pushnil(L);
            return 1;
        }

        IL2CPP::Il2CppClass* typeClass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "System", "Type");
        const IL2CPP::Il2CppType* objType = IL2CPP::class_get_type(typeClass);
        IL2CPP::Il2CppObject* objTypeReflection = IL2CPP::type_get_object(objType);

        void* typesArray = IL2CPP::array_new(typeClass, 1);
        uintptr_t typesHeader = (uintptr_t)IL2CPP::array_object_header_size();
        *(uintptr_t*)((uintptr_t)typesArray + typesHeader) = (uintptr_t)objTypeReflection;

        IL2CPP::Il2CppClass* reflMethodKlass = IL2CPP::object_get_class(reflMethod);
        IL2CPP::MethodInfo* makeGeneric = Reflections::FindMethod(reflMethodKlass, "MakeGenericMethod", 1);
        if (!makeGeneric)
        {
            M_LOGE("Cannot find MakeGenericMethod() on reflection method class!");
            lua_pushnil(L);
            return 1;
        }

        uintptr_t closedReflMethod = Invoke::CallMethodInternal(L, makeGeneric, [&](lua_State* L) {
            lua_pushlightuserdata(L, reflMethod);
            lua_pushlightuserdata(L, typesArray);
        });

        if (!closedReflMethod)
        {
            M_LOGE("MakeGenericMethod returned null!");
            lua_pushnil(L);
            return 1;
        }

        IL2CPP::MethodInfo* closedSetData = (IL2CPP::MethodInfo*)IL2CPP::method_get_from_reflection((void*)closedReflMethod);
        if (!closedSetData)
        {
            M_LOGE("il2cpp_method_get_from_reflection failed!");
            lua_pushnil(L);
            return 1;
        }

        M_LOGI("We are here");
        Invoke::CallMethodInternal(L, closedSetData, [&](lua_State* L) {
            lua_pushlightuserdata(L, textureInstance);
            lua_pushlightuserdata(L, objArray);
            lua_pushinteger(L, 0);
            lua_pushinteger(L, size);
        });

        lua_pushlightuserdata(L, textureInstance);
        return 1;
    }

    void RegisterCore(lua_State* L)
    {
        lua_pushcfunction(L, log_print);
        lua_setfield(L, -2, "log");

        lua_pushcfunction(L, lua_newString);
        lua_setfield(L, -2, "newString");

        lua_pushcfunction(L, lua_loadTexture);
        lua_setfield(L, -2, "loadTexture");
    }
}