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

        if (lua_islightuserdata(L, 2))
            graphicsDevice = lua_touserdata(L, 2);
        else if (lua_isuserdata(L, 2))
            graphicsDevice = *(void**)lua_touserdata(L, 2); 
         if (lua_isnumber(L, 2))
            graphicsDevice = (void*)(uintptr_t)lua_tointeger(L, 2);

        int width, height, channels;
        unsigned char* pixels = stbi_load(path, &width, &height, &channels, 4);
        if (!pixels)
        {
            M_LOGE("stbi_load failed: %s (%s)", path, stbi_failure_reason());
            lua_pushnil(L);
            return 1;
        }

        IL2CPP::Il2CppClass* xnaTextureClass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "Microsoft.Xna.Framework.Graphics", "Texture2D");
        IL2CPP::Il2CppClass* unityTextureClass = IL2CPP::Resolver::FindClass("UnityEngine.CoreModule", "UnityEngine", "Texture2D");

        if (!xnaTextureClass || !unityTextureClass)
        {
            stbi_image_free(pixels);
            lua_pushnil(L);
            return 1;
        }

        IL2CPP::MethodInfo* unityCtor4 = Reflections::FindMethod(unityTextureClass, ".ctor", 4);
        void* unityTextureInstance = IL2CPP::object_new(unityTextureClass);
            
        if (unityCtor4)
        {
            Invoke::CallMethodInternal(L, unityCtor4, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
                lua_pushinteger(L, width);
                lua_pushinteger(L, height);
                lua_pushinteger(L, 4);
                lua_pushinteger(L, 0);
            });
        }
        else
        {
            auto* unityCtor2 = Reflections::FindMethod(unityTextureClass, ".ctor", 2);
            Invoke::CallMethodInternal(L, unityCtor2, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
                lua_pushinteger(L, width);
                lua_pushinteger(L, height);
            });
        }

        IL2CPP::MethodInfo* getWritable = Reflections::FindMethod(unityTextureClass, "GetWritableImageData", 1);
        void* dataPtr = nullptr;
        if (getWritable)
        {
            dataPtr = (void*)Invoke::CallMethodInternal(L, getWritable, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
                lua_pushinteger(L, 0);
            });
        }

        if (!dataPtr)
        {
            M_LOGE("GetWritableImageData returned null! Cannot load pixels.");
            stbi_image_free(pixels);
            lua_pushnil(L);
            return 1;
        }

        auto* getRawSize = Reflections::FindMethod(unityTextureClass, "GetRawImageDataSize", 0);
        if (getRawSize)
        {
            uint64_t sz = (uint64_t)Invoke::CallMethodInternal(L, getRawSize, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
            });
            M_LOGI("Raw image data size: %llu (expected: %d)", (unsigned long long)sz, width * height * 4);
        }

        memcpy(dataPtr, pixels, width * height * 4);
        stbi_image_free(pixels);

        auto* apply = Reflections::FindMethod(unityTextureClass, "Apply", 0);
        if (apply)
        {
            Invoke::CallMethodInternal(L, apply, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
            });
        }
        else
        {
            auto* apply2 = Reflections::FindMethod(unityTextureClass, "Apply", 2);
            if (apply2)
            {
                Invoke::CallMethodInternal(L, apply2, [&](lua_State* L) {
                    lua_pushlightuserdata(L, unityTextureInstance);
                    lua_pushinteger(L, 0);
                    lua_pushinteger(L, 0);
                });
            }
        }

        auto* getWidth = Reflections::FindMethod(unityTextureClass, "get_width", 0);
        auto* getHeight = Reflections::FindMethod(unityTextureClass, "get_height", 0);
        if (getWidth && getHeight)
        {
            int uw = (int)Invoke::CallMethodInternal(L, getWidth, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
            });
            int uh = (int)Invoke::CallMethodInternal(L, getHeight, [&](lua_State* L) {
                lua_pushlightuserdata(L, unityTextureInstance);
            });
            M_LOGI("Unity texture after Apply: %dx%d", uw, uh);
        }

        auto* xnaCtor1 = Reflections::FindMethod(xnaTextureClass, ".ctor", 1);
        void* xnaTextureInstance = nullptr;

        if (xnaCtor1)
        {
            xnaTextureInstance = IL2CPP::object_new(xnaTextureClass);
            Invoke::CallMethodInternal(L, xnaCtor1, [&](lua_State* L) {
                lua_pushlightuserdata(L, xnaTextureInstance);
                lua_pushlightuserdata(L, unityTextureInstance);
            });

            auto* unityTexField = Reflections::FindField(xnaTextureClass, "_unityTexture", false);
            if (unityTexField)
            {
                size_t off = IL2CPP::field_get_offset(unityTexField);
                void* setTex = *(void**)((char*)xnaTextureInstance + off);
                M_LOGI("After ctor(Texture2D), _unityTexture = %p", setTex);

                if (setTex == unityTextureInstance)
                {
                    M_LOGI("SUCCESS: ctor(Texture2D) accepts Unity Texture2D!");

                    auto* renderTexField = Reflections::FindField(xnaTextureClass, "_unityRenderTexture", false);
                    auto* alphaTexField  = Reflections::FindField(xnaTextureClass, "_unityAlphaTexture", false);
                    auto* palTexField    = Reflections::FindField(xnaTextureClass, "_unityPalTexture", false);

                    if (renderTexField) 
                    {
                        size_t off = IL2CPP::field_get_offset(renderTexField);
                        void* val = *(void**)((char*)xnaTextureInstance + off);

                        if (val != nullptr) 
                        {
                            *(void**)((char*)xnaTextureInstance + off) = nullptr;
                            M_LOGI("Nulled _unityRenderTexture (was %p)", val);
                        }
                    }
                    if (alphaTexField) 
                    {
                        size_t off = IL2CPP::field_get_offset(alphaTexField);
                        *(void**)((char*)xnaTextureInstance + off) = nullptr;
                    }
                    if (palTexField) 
                    {
                        size_t off = IL2CPP::field_get_offset(palTexField);
                        *(void**)((char*)xnaTextureInstance + off) = nullptr;
                    }

                    auto* textureLoadedField = Reflections::FindField(xnaTextureClass, "_textureLoaded", false);
                    if (textureLoadedField)
                    {
                        size_t off = IL2CPP::field_get_offset(textureLoadedField);
                        bool* loadedPtr = (bool*)((char*)xnaTextureInstance + off);
                        if (!*loadedPtr) 
                        {
                            *loadedPtr = true;
                            M_LOGI("Set _textureLoaded = true in success path");
                        }
                    }

                    lua_pushlightuserdata(L, xnaTextureInstance);
                    return 1;
                }
                else
                {
                    M_LOGW("ctor(Texture2D) did NOT set _unityTexture (got %p, expected %p). Using fallback...", setTex, unityTextureInstance);
                    xnaTextureInstance = nullptr; 
                }
            }
        }

        M_LOGI("Using fallback constructor...");
        auto* xnaCtor3 = Reflections::FindMethod(xnaTextureClass, ".ctor", 3);
        xnaTextureInstance = IL2CPP::object_new(xnaTextureClass);

        auto* renderTexField = Reflections::FindField(xnaTextureClass, "_unityRenderTexture", false);
        auto* alphaTexField  = Reflections::FindField(xnaTextureClass, "_unityAlphaTexture", false);
        auto* palTexField    = Reflections::FindField(xnaTextureClass, "_unityPalTexture", false);

        if (renderTexField) *(void**)((char*)xnaTextureInstance + IL2CPP::field_get_offset(renderTexField)) = nullptr;
        if (alphaTexField)  *(void**)((char*)xnaTextureInstance + IL2CPP::field_get_offset(alphaTexField))  = nullptr;
        if (palTexField)    *(void**)((char*)xnaTextureInstance + IL2CPP::field_get_offset(palTexField))    = nullptr;

        Invoke::CallMethodInternal(L, xnaCtor3, [&](lua_State* L) {
            lua_pushlightuserdata(L, xnaTextureInstance);
            lua_pushlightuserdata(L, graphicsDevice);
            lua_pushinteger(L, width);
            lua_pushinteger(L, height);
        });

        auto* textureLoadedField = Reflections::FindField(xnaTextureClass, "_textureLoaded", false);
        if (textureLoadedField)
        {
            size_t off = IL2CPP::field_get_offset(textureLoadedField);
            *(bool*)((char*)xnaTextureInstance + off) = true;
        }

        auto* unityTexFieldInfo = Reflections::FindField(xnaTextureClass, "_unityTexture", false);
        if (unityTexFieldInfo)
        {
            IL2CPP::field_set_object((IL2CPP::Il2CppObject*)xnaTextureInstance, unityTexFieldInfo, unityTextureInstance);
        }

        lua_pushlightuserdata(L, xnaTextureInstance);
        return 1;
    }

    void RegisterCore(lua_State* L)
    {
        stbi_set_flip_vertically_on_load(true);

        lua_pushcfunction(L, log_print);
        lua_setfield(L, -2, "log");

        lua_pushcfunction(L, lua_newString);
        lua_setfield(L, -2, "newString");

        lua_pushcfunction(L, lua_loadTexture);
        lua_setfield(L, -2, "loadTexture");
    }
}