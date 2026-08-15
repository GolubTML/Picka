#pragma once
#include <stdlib.h>
#include "Il2CppStructs.h"

namespace IL2CPP
{
    #define REGISTER_IL2CPP_METHOD(r, n, p) typedef r (*t_##n) p; extern t_##n n

    // Domain
    REGISTER_IL2CPP_METHOD(void*, domain_get, ());
    REGISTER_IL2CPP_METHOD(Il2CppAssembly**, domain_get_assemblies, (void* domain, size_t* size));
    REGISTER_IL2CPP_METHOD(Il2CppImage*, assembly_get_image, (void* assembly));

    // Classes
    REGISTER_IL2CPP_METHOD(Il2CppClass*, class_from_name, (Il2CppImage* image, const char* namezpace, const char* name));
    REGISTER_IL2CPP_METHOD(MethodInfo*, class_get_method_from_name, (void* klass, const char* name, int argsCount));
    REGISTER_IL2CPP_METHOD(MethodInfo*, class_get_methods, (Il2CppClass* klass, void** iter));
    REGISTER_IL2CPP_METHOD(void*, class_get_field_from_name, (void* klass, const char* name));
    REGISTER_IL2CPP_METHOD(int32_t, class_value_size, (void* klass, uint32_t* align));
    REGISTER_IL2CPP_METHOD(void*, class_get_fields, (void* klass, void** iter));
    REGISTER_IL2CPP_METHOD(Il2CppType*, class_get_type, (Il2CppClass* klass));
    REGISTER_IL2CPP_METHOD(Il2CppClass*, object_get_class, (void* instance));
    REGISTER_IL2CPP_METHOD(bool, class_is_valuetype, (const Il2CppClass* klass));
    REGISTER_IL2CPP_METHOD(Il2CppObject*, object_new, (const Il2CppClass* klass));
    REGISTER_IL2CPP_METHOD(uint32_t, object_header_size, ());
    
    // Method
    REGISTER_IL2CPP_METHOD(const Il2CppType*, method_get_param, (const MethodInfo* method, uint32_t index));
    REGISTER_IL2CPP_METHOD(const char*, method_get_name, (const MethodInfo* methodInfo));
    REGISTER_IL2CPP_METHOD(Il2CppObject*, method_get_object, (const MethodInfo* method, const Il2CppClass* refclass));
    REGISTER_IL2CPP_METHOD(MethodInfo*, method_get_from_reflection, (const void* methodReflection));

    // Fields
    REGISTER_IL2CPP_METHOD(const char*, field_get_name, (void* field));
    REGISTER_IL2CPP_METHOD(void*, get_static_field_data, (void* klass));
    REGISTER_IL2CPP_METHOD(void*, class_get_static_field_data, (const Il2CppClass* klass));
    REGISTER_IL2CPP_METHOD(size_t, field_get_offset, (void* field));
    REGISTER_IL2CPP_METHOD(void*, class_get_parent, (void* klass));

    REGISTER_IL2CPP_METHOD(void, field_get_value, (void* obj, void* field, void* value));
    REGISTER_IL2CPP_METHOD(void, field_set_value, (void* obj, void* field, void* value));
    REGISTER_IL2CPP_METHOD(void*, field_static_get_value, (void* field, void* value));
    REGISTER_IL2CPP_METHOD(void*, field_static_set_value, (void* field, void* value));
    REGISTER_IL2CPP_METHOD(Il2CppType*, field_get_type, (void* field));
    REGISTER_IL2CPP_METHOD(uint32_t, field_get_flags, (void* field));

    void field_set_object(Il2CppObject* obj, void* field, void* value);

    REGISTER_IL2CPP_METHOD(Il2CppClass*, class_from_type, (const Il2CppType* type));
    REGISTER_IL2CPP_METHOD(int, type_get_type, (const Il2CppType* type));
    REGISTER_IL2CPP_METHOD(Il2CppObject*, type_get_object, (const Il2CppType* type));

     // Strings and values
    REGISTER_IL2CPP_METHOD(void*, new_string, (const char* str));
    REGISTER_IL2CPP_METHOD(char*, string_to_utf8, (void* str));

    // Array
    REGISTER_IL2CPP_METHOD(uint32_t, array_length, (void* array));
    REGISTER_IL2CPP_METHOD(uint32_t, array_object_header_size, ());
    REGISTER_IL2CPP_METHOD(Il2CppArray*, array_new, (Il2CppClass* elementTypeInfo, il2cpp_array_size_t length));

    // Garbage Collector
    REGISTER_IL2CPP_METHOD(void, gc_wbarrier_set_field, (Il2CppObject* obj, void** targetAddr, void* object));

    #undef REGISTER_IL2CPP_METHOD

    uintptr_t GetIl2CppBase();
    void InitIl2CppAPI();
}