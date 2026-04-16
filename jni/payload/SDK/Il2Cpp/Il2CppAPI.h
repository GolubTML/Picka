#pragma once
#include <stdlib.h>

namespace IL2CPP
{
    #define REGISTER_IL2CPP_METHOD(r, n, p) typedef r (*t_##n) p; extern t_##n n

    // Domain
    REGISTER_IL2CPP_METHOD(void*, domain_get, ());
    REGISTER_IL2CPP_METHOD(void**, domain_get_assemblies, (void* domain, size_t* size));
    REGISTER_IL2CPP_METHOD(void*, assembly_get_image, (void* assembly));
    REGISTER_IL2CPP_METHOD(void*, class_from_name, (void* image, const char* namezpace, const char* name));
    REGISTER_IL2CPP_METHOD(void*, class_get_method_from_name, (void* klass, const char* name, int argsCount));
    
    // Classes
    REGISTER_IL2CPP_METHOD(void*, class_get_fields, (void* klass, void** iter));
    REGISTER_IL2CPP_METHOD(void*, get_static_field_data, (void* klass));
    REGISTER_IL2CPP_METHOD(const char*, field_get_name, (void* field));
    REGISTER_IL2CPP_METHOD(size_t, field_get_offset, (void* field));
    REGISTER_IL2CPP_METHOD(void*, class_get_parent, (void* klass));

    // Strings and values
    REGISTER_IL2CPP_METHOD(void, field_get_value, (void* obj, void* field, void* value));
    REGISTER_IL2CPP_METHOD(void, field_set_value, (void* obj, void* field, void* value));
    REGISTER_IL2CPP_METHOD(void*, field_static_get_value, (void* field, void* value));
    REGISTER_IL2CPP_METHOD(uint32_t, field_get_flags, (void* field));

    REGISTER_IL2CPP_METHOD(void*, new_string, (const char* str));
    REGISTER_IL2CPP_METHOD(char*, string_to_utf8, (void* str));

    #undef REGISTER_IL2CPP_METHOD

    void InitIl2CppAPI();
}