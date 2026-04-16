#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>

#include "SDK/hook.h"
#include "log.h"
#include "crash.h"
#include "imgui.h"
#include "SDK/SDK.h"
#include "SDK/Il2Cpp/Il2CppAPI.h"
#include "backends/imgui_impl_opengl3.h"


#include "test_mods/no_damage.h"
#include "test_mods/chat_menu.h"
#include <dlfcn.h>

void InitAllMods()
{
    std::vector<HookTarget> hooks = 
    {
        { "Assembly-CSharp", "Terraria", "Player", "Hurt", -1, (void*)my_Player_Hurt, (void**)&original_Player_Hurt },
        { "Assembly-CSharp", "Terraria.Chat", "ChatCommandProcessor", "ProcessIncomingMessage", -1, (void*)my_ProcessIncomingMessage, (void**)&orig_ProcessIncomingMessage }
    };

    InstallDynamicHooks(hooks);
}

typedef int (*il2cpp_init_fn)(const char* domain_name);
il2cpp_init_fn original_il2cpp_init = NULL;

int my_il2cpp_init(const char* domain_name) 
{
    int result = ((il2cpp_init_fn)original_il2cpp_init)(domain_name);

    LOGI("IL2CPP is ready! Domain created. Starting dynamic hooks...");

    IL2CPP::InitIl2CppAPI();

    uintptr_t il2cpp_lib = get_base_addres("libil2cpp.so");

    SDK::Init(il2cpp_lib);
    InitAllMods();

    return result;
}

extern "C" __attribute__((visibility("default")))
void payload_init()
{
    __android_log_print(ANDROID_LOG_INFO, "Payload", "payload_init called!");
    setup_crash_handler();
    LOGI("Payload API init!");

    void* handle = dlopen("libil2cpp.so", RTLD_NOW);
    void* init_addr = dlsym(handle, "il2cpp_init");

    if (init_addr)
    {
        uintptr_t original = 0;

        hook_function((uintptr_t)init_addr, (uintptr_t)my_il2cpp_init, &original, 4);
        original_il2cpp_init = (il2cpp_init_fn)original;
        LOGI("Hooked il2cpp_init at %p", init_addr);
    }
    
    
    // void* vk = dlopen("libvulkan.so", RTLD_NOW);
    // if (vk) 
    // {
    //     void* present = dlsym(vk, "vkQueuePresentKHR");
    // 
    //     LOGI("Vulkan available, vkQueuePresentKHR: %p", present);
    // } else 
    //    {
    //     LOGI("Vulkan not available: %s", dlerror());
    // }

    // yeah, we have vulkan, but i dont wanna use it tbh
}