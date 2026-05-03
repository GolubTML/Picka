#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>

#include "SDK/hook.h"
#include "Menu/NativeMenu.h"
#include "Menu/FloatButton.h"
#include "Menu/ModMenu.h"
#include "log.h"
#include "crash.h"
#include "SDK/SDK.h"
#include "SDK/Il2Cpp/Il2CppAPI.h"
#include "SDK/Il2Cpp/Il2CppResolver.h"


#include "test_mods/no_damage.h"
#include "test_mods/chat_menu.h"
#include <dlfcn.h>

JavaVM* g_vm = nullptr;

extern "C" 
{
    JNIEXPORT void JNICALL __attribute__((visibility("default")))
    Java_com_picka_tools_FloatButtonHelper_onClickNative(JNIEnv* env, jclass clazz) 
    {
        LOGI("JNI: Click started");
        if (Menu::instance == nullptr) 
        {
            LOGI("ERR: Menu::instance is NULL!");
            return;
        }
        
        Menu::instance->menu->Show();
    }
}

void RegisterNativeMethods(JNIEnv* env) 
{
    jclass clazz = env->FindClass("com/picka/tools/FloatButtonHelper");
    if (!clazz) {
        LOGI("Native: Could not find FloatButtonHelper class for registration");
        return;
    }

    JNINativeMethod methods[] = {
        {(char*)"onClickNative", (char*)"()V", (void*)Java_com_picka_tools_FloatButtonHelper_onClickNative}
    };

    if (env->RegisterNatives(clazz, methods, 1) < 0) {
        LOGI("Native: RegisterNatives failed!");
    } else {
        LOGI("Native: RegisterNatives SUCCESS!");
    }
}

void InitAllMods()
{
    std::vector<HookTarget> hooks = 
    {
        { "Assembly-CSharp", "Terraria", "Main", "Update", 1, (void*)Menu::my_Update, (void**)&Menu::orig_Main_Update },
        { "Assembly-CSharp", "Terraria", "Player", "Hurt", -1, (void*)my_Player_Hurt, (void**)&original_Player_Hurt },
        { "Assembly-CSharp", "Terraria.Chat", "ChatCommandProcessor", "ProcessIncomingMessage", -1, (void*)my_ProcessIncomingMessage, (void**)&orig_ProcessIncomingMessage },
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
    Menu::InitUnityInput();

    LOGI("Init Unity input/screen");

    typedef jint (*GetCreatedVMs_t)(JavaVM**, jsize, jsize*);
    GetCreatedVMs_t fnGetCreatedVMs = (GetCreatedVMs_t)dlsym(RTLD_DEFAULT, "JNI_GetCreatedJavaVMs");

    if (!fnGetCreatedVMs) 
    {
        void* libart = dlopen("libart.so", RTLD_NOW);

        if (libart) 
        {
            fnGetCreatedVMs = (GetCreatedVMs_t)dlsym(libart, "JNI_GetCreatedJavaVMs");
        }

        if (!fnGetCreatedVMs) 
        {
            void* libnh = dlopen("libnativehelper.so", RTLD_NOW);

            if (libnh) 
            {
                fnGetCreatedVMs = (GetCreatedVMs_t)dlsym(libnh, "JNI_GetCreatedJavaVMs");
            }
        }
    }

    if (fnGetCreatedVMs) 
    {
        JavaVM* vms[1]; // idk, but JNI_OnLoad doesnt work. I know why, it because of my libloader.so
        jsize num_vms;

        if (fnGetCreatedVMs(vms, 1, &num_vms) == JNI_OK && num_vms > 0) 
        {
            g_vm = vms[0];
            LOGI("Got JavaVM via dlsym: %p", g_vm);
        }
    } 
    else 
    {
        LOGI("Could not find JNI_GetCreatedJavaVMs symbol!");
    }

    JNIEnv* env = nullptr;

    if (g_vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK)
    {
        jclass upClass = env->FindClass("com/unity3d/player/UnityPlayer");
    
        if (upClass) 
        {
            jfieldID caField = env->GetStaticFieldID(upClass, "currentActivity", "Landroid/app/Activity;");
            jobject currentActivity = env->GetStaticObjectField(upClass, caField);

            if (currentActivity) 
            {
                LOGI("Found currentActivity, creating button...");

                RegisterNativeMethods(env);
                Menu::instance = std::make_unique<Menu::ModMenu>(env, currentActivity);

                LOGI("Native menu has been initilized!");
            } 
            else 
            {
                LOGI("Failed to get currentActivity");
            }
        } 
        else 
        {
            LOGI("Failed to find UnityPlayer class");
        }
    }
    else 
    {
        LOGI("Failed to get JNIEnv from JavaVM");
    }
    
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
}