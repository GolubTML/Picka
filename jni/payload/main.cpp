#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>

#include "SDK/hook.h"
#include "Menu/NativeMenu.h"
#include "log.h"
#include "crash.h"
#include "SDK/SDK.h"
#include "SDK/Il2Cpp/Il2CppAPI.h"
#include "SDK/Il2Cpp/Il2CppResolver.h"


#include "test_mods/no_damage.h"
#include "test_mods/chat_menu.h"
#include <dlfcn.h>

JavaVM* g_vm = nullptr;
std::unique_ptr<NativeMenu> menu;

typedef void (*t_Main_Update)(void* gameTime);
t_Main_Update orig_Main_Update;

void my_Update(void* gameTime)
{
    orig_Main_Update(gameTime);

    if (menu)
        menu->UpdateButtons();
}

typedef int (*t_NewItem)(int, int, int, int, int, int, bool, int, bool, void*);

void* GetLocalPlayer2()
{
    static void* main_klass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "Terraria", "Main");
    if (!main_klass) return nullptr;

    static void* myPlayer_field = IL2CPP::Resolver::FindField(main_klass, "myPlayer");
    static void* player_field = IL2CPP::Resolver::FindStaticField(main_klass, "player");

    LOGI("Fields: %p, %p", myPlayer_field, player_field);
    void* static_data = IL2CPP::get_static_field_data(main_klass);
    
    int myPlayerIdx = *(int*)((uintptr_t)static_data + 0x424); // wtf, why i can't allocate myPlayer field? but it static in the class..

    void* playerArray = nullptr;
    IL2CPP::field_static_get_value(player_field, &playerArray);

    if (!playerArray || myPlayerIdx < 0) return nullptr;

    void** items = (void**)((uintptr_t)playerArray + 0x20); 
    return items[myPlayerIdx];
}

void SpawItem(int itemID)
{
    void* player = GetLocalPlayer2();
    if (!player) return;

    static void* method_ptr = IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Item", "NewItem", 9);
    if (!method_ptr) return;

    auto NewItem = (t_NewItem)(*(void**)method_ptr);

    static void* posField = IL2CPP::Resolver::FindField(*(void**)player, "position");
    Vector2 pos = IL2CPP::Resolver::GetFieldValue<Vector2>(player, posField);

    NewItem((int)pos.x, (int)pos.y, 0, 0, itemID, 1, false, 0, false, nullptr);
}

void SetGhostPlayer()
{
    void* player = GetLocalPlayer2();
    if (!player) return;

    static void* ghostField = IL2CPP::Resolver::FindField(*(void**)player, "ghost");

    if (ghostField)
    {
        bool currentState = IL2CPP::Resolver::GetFieldValue<bool>(player, ghostField);

        bool newState = !currentState;

        IL2CPP::Resolver::SetFieldValue<bool>(player, ghostField, newState);

        if (newState) 
            SDK::Chat("Ghost Mode: [ON]", {0, 255, 0});
        else 
            SDK::Chat("Ghost Mode: [OFF]", {255, 0, 0});
    }
}

void InitAllMods()
{
    std::vector<HookTarget> hooks = 
    {
        { "Assembly-CSharp", "Terraria", "Main", "Update", 1, (void*)my_Update, (void**)&orig_Main_Update },
        { "Assembly-CSharp", "Terraria", "Player", "Hurt", -1, (void*)my_Player_Hurt, (void**)&original_Player_Hurt },
        { "Assembly-CSharp", "Terraria.Chat", "ChatCommandProcessor", "ProcessIncomingMessage", -1, (void*)my_ProcessIncomingMessage, (void**)&orig_ProcessIncomingMessage },
    };
    
    InstallDynamicHooks(hooks);
}

typedef int (*il2cpp_init_fn)(const char* domain_name);
il2cpp_init_fn original_il2cpp_init = NULL;

// extern "C" __attribute__((visibility("default"))) jint JNI_OnLoad(JavaVM* vm, void* reserved) 
// {
//     g_vm = vm;
//     LOGI("\tJNI_OnLoad called. JavaVM saved at %p", g_vm);
//     return JNI_VERSION_1_6;
// }

int my_il2cpp_init(const char* domain_name) 
{
    int result = ((il2cpp_init_fn)original_il2cpp_init)(domain_name);

    LOGI("IL2CPP is ready! Domain created. Starting dynamic hooks...");

    IL2CPP::InitIl2CppAPI();

    uintptr_t il2cpp_lib = get_base_addres("libil2cpp.so");

    SDK::Init(il2cpp_lib);
    InitAllMods();

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

                menu = std::make_unique<NativeMenu>(env, currentActivity);

                menu->AddButton("Ghost Mode", []() { SetGhostPlayer(); });
                menu->AddInputButton("Type Item ID", "Spawn", [](std::string str) 
                { 
                    try
                    {
                        int itemID = std::stoi(str);
                        SpawItem(itemID);
                        
                        SDK::Chat("Spawned item from menu!", {0, 255, 0});
                    }   
                    catch (...)
                    {
                        SDK::Chat("Invalid item ID! Example: 4956", {255, 0, 0});
                    }
                });
                menu->AddButton("BS", []() { SDK::Chat("BS", {255, 0, 0}); });
                
                menu->Show();

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