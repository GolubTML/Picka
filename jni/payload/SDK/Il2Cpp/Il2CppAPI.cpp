#include "Il2CppAPI.h"
#include "log.h"
#include <dlfcn.h>

namespace IL2CPP
{
    // TODO: need to refactor this mess
    t_domain_get domain_get = NULL;
    t_domain_get_assemblies domain_get_assemblies = NULL;
    t_assembly_get_image assembly_get_image = NULL;
    t_class_from_name class_from_name = NULL;
    t_class_get_name class_get_name = NULL;
    t_class_is_enum class_is_enum = NULL;
    t_class_get_methods class_get_methods = NULL;
    t_class_get_method_from_name class_get_method_from_name = NULL;
    t_class_get_field_from_name class_get_field_from_name = NULL;
    t_class_value_size class_value_size = NULL;
    
    t_class_get_fields class_get_fields = NULL;
    t_get_static_field_data get_static_field_data = NULL;
    t_class_get_static_field_data class_get_static_field_data = NULL;
    t_field_get_name field_get_name = NULL;
    t_field_get_offset field_get_offset = NULL;
    t_class_get_parent class_get_parent = NULL;
    t_object_get_class object_get_class = NULL;
    t_class_is_valuetype class_is_valuetype = NULL;
    t_class_get_type class_get_type = NULL;
    t_object_new object_new = NULL;
    t_object_header_size object_header_size = NULL;

    t_method_get_param method_get_param = NULL;
    t_method_get_name method_get_name = NULL;
    t_method_get_object method_get_object = NULL;
    t_method_get_from_reflection method_get_from_reflection = NULL;

    t_field_get_value field_get_value = NULL;
    t_field_set_value field_set_value = NULL;
    t_field_static_get_value field_static_get_value = NULL;
    t_field_static_set_value field_static_set_value = NULL;
    t_field_get_type field_get_type = NULL;
    t_field_get_flags field_get_flags = NULL;

    t_class_from_type class_from_type = NULL;
    t_type_get_type type_get_type = NULL;
    t_type_get_object type_get_object = NULL;

    t_new_string new_string = NULL;
    t_string_to_utf8 string_to_utf8 = NULL;

    t_array_length array_length = NULL;
    t_array_object_header_size array_object_header_size = NULL;
    t_array_new array_new = NULL;

    t_gc_wbarrier_set_field gc_wbarrier_set_field = NULL;
    
    void field_set_object(Il2CppObject* obj, void* field, void* value)
    {
        size_t offset = IL2CPP::field_get_offset(field);
        void** fieldPtr = (void**)((char*)obj + offset);
        *fieldPtr = value;

        gc_wbarrier_set_field(obj, fieldPtr, value);
    }

    uintptr_t GetIl2CppBase()
    {
        Dl_info info;

        if (dladdr(&domain_get, &info) && info.dli_fbase)
            return (uintptr_t)info.dli_fbase;
        
        return 0;
    }

    void InitIl2CppAPI()
    {
        void* handle = dlopen("libil2cpp.so", RTLD_NOW);

        domain_get = (t_domain_get)dlsym(handle, "il2cpp_domain_get");
        domain_get_assemblies = (t_domain_get_assemblies)dlsym(handle, "il2cpp_domain_get_assemblies");
        assembly_get_image = (t_assembly_get_image)dlsym(handle, "il2cpp_assembly_get_image");
        class_from_name = (t_class_from_name)dlsym(handle, "il2cpp_class_from_name");
        class_get_name = (t_class_get_name)dlsym(handle, "il2cpp_class_get_name");
        class_is_enum = (t_class_is_enum)dlsym(handle, "il2cpp_class_is_enum");
        class_get_methods = (t_class_get_methods)dlsym(handle, "il2cpp_class_get_methods");
        class_get_method_from_name = (t_class_get_method_from_name)dlsym(handle, "il2cpp_class_get_method_from_name");
        class_get_field_from_name = (t_class_get_field_from_name)dlsym(handle, "il2cpp_class_get_field_from_name");
        class_get_parent = (t_class_get_parent)dlsym(handle, "il2cpp_class_get_parent");
        class_get_type = (t_class_get_type)dlsym(handle, "il2cpp_class_get_type");
        object_get_class = (t_object_get_class)dlsym(handle, "il2cpp_object_get_class");
        class_value_size = (t_class_value_size)dlsym(handle, "il2cpp_class_value_size");
        class_is_valuetype = (t_class_is_valuetype)dlsym(handle, "il2cpp_class_is_valuetype");
        object_new = (t_object_new)dlsym(handle, "il2cpp_object_new");
        object_header_size = (t_object_header_size)dlsym(handle, "il2cpp_object_header_size");

        method_get_param = (t_method_get_param)dlsym(handle, "il2cpp_method_get_param");
        method_get_name = (t_method_get_name)dlsym(handle, "il2cpp_method_get_name");
        method_get_object = (t_method_get_object)dlsym(handle, "il2cpp_method_get_object");
        method_get_from_reflection = (t_method_get_from_reflection)dlsym(handle, "il2cpp_method_get_from_reflection");
        
        class_get_fields = (t_class_get_fields)dlsym(handle, "il2cpp_class_get_fields");
        get_static_field_data = (t_get_static_field_data)dlsym(handle, "il2cpp_class_get_static_field_data");
        class_get_static_field_data = (t_class_get_static_field_data)dlsym(handle, "il2cpp_class_get_static_field_data");
        field_get_name = (t_field_get_name)dlsym(handle, "il2cpp_field_get_name");
        field_get_offset = (t_field_get_offset)dlsym(handle, "il2cpp_field_get_offset");

        field_get_value = (t_field_get_value)dlsym(handle, "il2cpp_field_get_value");
        field_set_value = (t_field_set_value)dlsym(handle, "il2cpp_field_set_value");
        field_static_get_value = (t_field_static_get_value)dlsym(handle, "il2cpp_field_static_get_value");
        field_static_set_value = (t_field_static_set_value)dlsym(handle, "il2cpp_field_static_set_value");
        field_get_type = (t_field_get_type)dlsym(handle, "il2cpp_field_get_type");
        field_get_flags = (t_field_get_flags)dlsym(handle, "il2cpp_field_get_flags");

        class_from_type = (t_class_from_type)dlsym(handle, "il2cpp_class_from_type");
        type_get_type = (t_type_get_type)dlsym(handle, "il2cpp_type_get_type");
        type_get_object = (t_type_get_object)dlsym(handle, "il2cpp_type_get_object");

        new_string = (t_new_string)dlsym(handle, "il2cpp_string_new");
        string_to_utf8 = (t_string_to_utf8)dlsym(handle, "mono_string_to_utf8_checked");

        if (!string_to_utf8) 
        {
            LOGD("Attempting fallback for string_to_utf8...");
            string_to_utf8 = (t_string_to_utf8)dlsym(handle, "il2cpp_string_to_utf8");
        }

        if (!string_to_utf8) 
        {
            LOGE("CRITICAL: il2cpp_string_to_utf8 TOTALLY not found!");
        } 
        else 
        {
            LOGI("Success: il2cpp_string_to_utf8 found at %p", (void*)string_to_utf8);
        }

        array_length = (t_array_length)dlsym(handle, "il2cpp_array_length");
        array_object_header_size = (t_array_object_header_size)dlsym(handle, "il2cpp_array_object_header_size");
        array_new = (t_array_new)dlsym(handle, "il2cpp_array_new");

        gc_wbarrier_set_field = (t_gc_wbarrier_set_field)dlsym(handle, "il2cpp_gc_wbarrier_set_field");
    }
}