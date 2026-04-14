#include "Il2CppAPI.h"
#include <dlfcn.h>

namespace IL2CPP
{
    t_il2cpp_domain_get domain_get = NULL;
    t_il2cpp_domain_get_assemblies domain_get_assemblies = NULL;
    t_il2cpp_assembly_get_image assembly_get_image = NULL;
    t_il2cpp_class_from_name class_from_name = NULL;
    t_il2cpp_class_get_method_from_name il2cpp_class_get_method_from_name = NULL;

    t_il2cpp_string_new new_string = NULL;
    t_il2cpp_string_to_utf8 string_to_utf8 = NULL;

    void InitIl2CppAPI()
    {
        void* handle = dlopen("libil2cpp.so", RTLD_NOW);

        domain_get = (t_il2cpp_domain_get)dlsym(handle, "il2cpp_domain_get");
        domain_get_assemblies = (t_il2cpp_domain_get_assemblies)dlsym(handle, "il2cpp_domain_get_assemblies");
        assembly_get_image = (t_il2cpp_assembly_get_image)dlsym(handle, "il2cpp_assembly_get_image");
        class_from_name = (t_il2cpp_class_from_name)dlsym(handle, "il2cpp_class_from_name");
        il2cpp_class_get_method_from_name = (t_il2cpp_class_get_method_from_name)dlsym(handle, "il2cpp_class_get_method_from_name");

        new_string = (t_il2cpp_string_new)dlsym(handle, "il2cpp_string_new");
        string_to_utf8 = (t_il2cpp_string_to_utf8)dlsym(handle, "il2cpp_string_to_utf8");
    }

    void* FindMethod(const char* assemblyName, const char* namezpace, const char* klassName, const char* methodName, int argsCount)
    {
        size_t size;
        void** assemblies = domain_get_assemblies(domain_get(), &size);

        for (size_t i = 0; i < size; ++i)
        {
            void* image = assembly_get_image(assemblies[i]);

            void* klass = class_from_name(image, namezpace, klassName);
            
            if (klass)
            {
                void* method = il2cpp_class_get_method_from_name(klass, methodName, argsCount);

                if (method)
                {
                    return method;
                }
            }
        }

        return nullptr;
    }
}

