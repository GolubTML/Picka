#pragma once
#include <stdlib.h>

namespace IL2CPP
{
    typedef void*  (*t_il2cpp_domain_get)();
    typedef void** (*t_il2cpp_domain_get_assemblies)(void* domain, size_t* size);
    typedef void*  (*t_il2cpp_assembly_get_image)(void* assembly);
    typedef void*  (*t_il2cpp_class_from_name)(void* image, const char* namezpace, const char* name);
    typedef void* (*t_il2cpp_class_get_method_from_name)(void* klass, const char* name, int argsCount);

    typedef void* (*t_il2cpp_string_new)(const char* str);
    typedef char* (*t_il2cpp_string_to_utf8)(void* str);

    extern t_il2cpp_domain_get domain_get;
    extern t_il2cpp_domain_get_assemblies domain_get_assemblies;
    extern t_il2cpp_assembly_get_image assembly_get_image;
    extern t_il2cpp_class_from_name class_from_name;
    extern t_il2cpp_class_get_method_from_name il2cpp_class_get_method_from_name;

    extern t_il2cpp_string_new new_string;
    extern t_il2cpp_string_to_utf8 string_to_utf8;

    void InitIl2CppAPI();

    void* FindMethod(const char* assemblyName, const char* namezpace, const char* klassName, const char* methodName, int argsCount);
}