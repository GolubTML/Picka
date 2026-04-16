#include "Il2CppAPI.h"
#include "log.h"
#include <dlfcn.h>

namespace IL2CPP
{
    t_domain_get domain_get = NULL;
    t_domain_get_assemblies domain_get_assemblies = NULL;
    t_assembly_get_image assembly_get_image = NULL;
    t_class_from_name class_from_name = NULL;
    t_class_get_method_from_name class_get_method_from_name = NULL;
    
    t_class_get_fields class_get_fields = NULL;
    t_get_static_field_data get_static_field_data = NULL;
    t_field_get_name field_get_name = NULL;
    t_field_get_offset field_get_offset = NULL;
    t_class_get_parent class_get_parent = NULL;

    t_field_get_value field_get_value = NULL;
    t_field_set_value field_set_value = NULL;
    t_field_static_get_value field_static_get_value = NULL;
    t_field_get_flags field_get_flags = NULL;

    t_new_string new_string = NULL;
    t_string_to_utf8 string_to_utf8 = NULL;

    void InitIl2CppAPI()
    {
        void* handle = dlopen("libil2cpp.so", RTLD_NOW);

        domain_get = (t_domain_get)dlsym(handle, "il2cpp_domain_get");
        domain_get_assemblies = (t_domain_get_assemblies)dlsym(handle, "il2cpp_domain_get_assemblies");
        assembly_get_image = (t_assembly_get_image)dlsym(handle, "il2cpp_assembly_get_image");
        class_from_name = (t_class_from_name)dlsym(handle, "il2cpp_class_from_name");
        class_get_method_from_name = (t_class_get_method_from_name)dlsym(handle, "il2cpp_class_get_method_from_name");
        class_get_parent = (t_class_get_parent)dlsym(handle, "il2cpp_class_get_parent");

        class_get_fields = (t_class_get_fields)dlsym(handle, "il2cpp_class_get_fields");
        get_static_field_data = (t_get_static_field_data)dlsym(handle, "il2cpp_class_get_static_field_data");
        field_get_name = (t_field_get_name)dlsym(handle, "il2cpp_field_get_name");
        field_get_offset = (t_field_get_offset)dlsym(handle, "il2cpp_field_get_offset");

        field_get_value = (t_field_get_value)dlsym(handle, "il2cpp_field_get_value");
        field_set_value = (t_field_set_value)dlsym(handle, "il2cpp_field_set_value");
        field_static_get_value = (t_field_static_get_value)dlsym(handle, "il2cpp_field_static_get_value");
        field_get_flags = (t_field_get_flags)dlsym(handle, "il2cpp_field_get_flags");

        new_string = (t_new_string)dlsym(handle, "il2cpp_string_new");
        string_to_utf8 = (t_string_to_utf8)dlsym(handle, "mono_string_to_utf8_checked");

        if (!string_to_utf8) 
        {
            LOGI("Attempting fallback for string_to_utf8...");
            string_to_utf8 = (t_string_to_utf8)dlsym(handle, "il2cpp_string_to_utf8");
        }

        if (!string_to_utf8) 
        {
            LOGI("CRITICAL: il2cpp_string_to_utf8 TOTALLY not found!");
        } 
        else 
        {
            LOGI("Success: il2cpp_string_to_utf8 found at %p", (void*)string_to_utf8);
        }
    }
}