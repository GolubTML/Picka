#include "Il2CppResolver.h"
#include "log.h"

namespace IL2CPP
{
    void* Resolver::FindClass(const char* assemlyName, const char* namezpace, const char* klassName)
    {
        size_t size;
        void** assemblies = domain_get_assemblies(domain_get(), &size);

        for (size_t i = 0; i < size; ++i)
        {
            void* image = assembly_get_image(assemblies[i]);

            void* klass = class_from_name(image, namezpace, klassName);

            if (klass)
            {
                return klass;
            }
        }

        return nullptr;
    }

    void* Resolver::FindMethod(const char* assemblyName, const char* namezpace, const char* klassName, const char* methodName, int argsCount)
    {
        size_t size;
        void** assemblies = domain_get_assemblies(domain_get(), &size);

        for (size_t i = 0; i < size; ++i)
        {
            void* image = assembly_get_image(assemblies[i]);

            void* klass = class_from_name(image, namezpace, klassName);
            
            if (klass)
            {
                void* method = class_get_method_from_name(klass, methodName, argsCount);

                if (method)
                {
                    return method;
                }
            }
        }

        return nullptr;
    }

    void* Resolver::FindField(void* klass, const char* fieldName)
    {
        void* current = klass;

        while (current)
        {
            void* iter = NULL;
            void* field;

            while ((field = class_get_fields(current, &iter)))
            {
                if (strcmp(field_get_name(field), fieldName) == 0)
                {
                    return field;
                }
            }

            current = class_get_parent(current);
        }

        return nullptr;
    }

    void* Resolver::FindStaticField(void* klass, const char* fieldName)
    {
        void* current = klass;

        while (current)
        {
            void* iter = NULL;
            void* field;

            while ((field = class_get_fields(current, &iter)))
            {
                const char* internalName = field_get_name(field);
                LOGI("Class Main has field: %s", internalName);

                if (strcmp(field_get_name(field), fieldName) == 0)
                {
                    uint32_t flags = field_get_flags(field);
                    
                    if (flags & IL2CPP_STATIC_FIELD)
                        return field;
                }
            }

            current = class_get_parent(current);
        }

        return nullptr;
    }

    size_t Resolver::GetFieldOffset(void* klass, const char* fieldName)
    {
        void* currentKlass = klass;

        while (currentKlass)
        {
            void* iter = nullptr;
            void* field;

            while ((field = class_get_fields(currentKlass, &iter)))
            {
                const char* name = field_get_name(field);

                if (strcmp(field_get_name(field), fieldName) == 0)
                {
                    return field_get_offset(field);
                }
            }

            currentKlass = class_get_parent(currentKlass);
        }

        LOGI("Field %s not found!", fieldName);
        return 0;
    }

    std::string Resolver::GetString(void* il2cpp_string)
    {
        if (!il2cpp_string || (uintptr_t)il2cpp_string < 0x1000000) return "";

        int32_t length = *(int32_t*)((uintptr_t)il2cpp_string + 0x10); // here, x10 is offset is lenght of line
        if (length <= 0 || length > 2048) return "";

        char16_t* wide_chars = (char16_t*)((uintptr_t)il2cpp_string + 0x14); // x14 is start of array UTF-16

        std::string result;
        result.reserve(length);

        for (int i = 0; i < length; ++i)
            result += (char)wide_chars[i];

        return result;
    }
}