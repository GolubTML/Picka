#include "Il2CppAPI.h"
#include <string>

namespace IL2CPP
{
    #define IL2CPP_STATIC_FIELD 0x0010

    class Resolver
    {
        public:
            static void* FindClass(const char* assemlyName, const char* namezpace, const char* klassName);
            static void* FindMethod(const char* assemblyName, const char* namezpace, const char* klassName, const char* methodName, int argsCount);
            static void* FindField(void* klass, const char* fieldName);
            static void* FindStaticField(void* klass, const char* fieldName);

            template <typename T>
            static T GetFieldValue(void* instance, void* fieldInfo)
            {
                T val;
                field_get_value(instance, fieldInfo, &val);
                return val;
            }

            template <typename T>
            static void SetFieldValue(void* instance, void* fieldInfo, T value)
            {   
                field_set_value(instance, fieldInfo, &value);
            }

            static size_t GetFieldOffset(void* klass, const char* fieldName);
            static std::string GetString(void* il2cpp_string); // idk why, but mono_string_to_utf8_checked doesnt work, so, let's make our own
    };
}