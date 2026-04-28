#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>

#include "SDK/hook.h"
#include "Menu/NativeMenu.h"
#include "Menu/FloatButton.h"
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
std::unique_ptr<FloatButton> floatBtn;

typedef void (*t_Main_Update)(void* gameTime);
t_Main_Update orig_Main_Update;

typedef Vector3 (*t_get_mousePosition)();
typedef bool (*t_GetMouseButton)(int);
typedef int (*t_get_ScreenValue)();

t_get_mousePosition get_mousePosition = nullptr;
t_GetMouseButton get_mouseButton = nullptr;
t_get_ScreenValue get_screenWidth = nullptr;
t_get_ScreenValue get_screenHeight = nullptr;

// I will fix all of this shit, i promise

void InitUnityInput()
{
    void* inputKlass = IL2CPP::Resolver::FindClass("UnityEngine", "UnityEngine", "Input");
    void* screenKlass = IL2CPP::Resolver::FindClass("UnityEngine", "UnityEngine", "Screen"); // TODO: make IL2CPP::Resolver::FindMethod with class it self, not with assemblies, namezpace, klass.. 

    if (inputKlass) 
    {
        void* mousePos_ptr = IL2CPP::Resolver::FindMethod("UnityEngine", "UnityEngine", "Input", "get_mousePosition", 0);
        void* mouseBtn_ptr = IL2CPP::Resolver::FindMethod("UnityEngine", "UnityEngine", "Input", "GetMouseButton", 1);

        if (mousePos_ptr)
            get_mousePosition = (t_get_mousePosition)(*(void**)mousePos_ptr);

        if (mouseBtn_ptr)
            get_mouseButton = (t_GetMouseButton)(*(void**)mouseBtn_ptr);
    }

    if (screenKlass) 
    {
        void* screenWidth_ptr = IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Main", "get_screenWidth", 0);
        void* screenHeight_ptr = IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Main", "get_screenHeight", 0);

        if (screenWidth_ptr)
            get_screenWidth = (t_get_ScreenValue)(*(void**)screenWidth_ptr);

        if (screenHeight_ptr)
            get_screenHeight = (t_get_ScreenValue)(*(void**)screenHeight_ptr);
    }

    LOGI("Unity Input Resolved: %p, %p", get_mousePosition, get_mouseButton);
    LOGI("Unity Screen Resolver: %p, %p", get_screenWidth, get_screenHeight);
}

void my_Update(void* gameTime)
{
    orig_Main_Update(gameTime);

    if (floatBtn && get_mousePosition && get_mouseButton)
    {
        static bool lastState = false;
        bool currentState = get_mouseButton(0);

        Vector3 mousePos = get_mousePosition();

        int screenH = get_screenHeight ? get_screenHeight() : 1080;
        int rawX = (int)mousePos.x;
        int rawY = screenH - (int)mousePos.y;

        int action = -1;

        if (currentState && !lastState) 
        {
            action = 0;
        } 
        else if (currentState && lastState) 
        {
            action = 2; 
        } 
        else if (!currentState && lastState) 
        {
            action = 1; 
        }

        if (action != -1) 
        {
            floatBtn->OnTouch(action, rawX, rawY);
        }

        lastState = currentState;
    }
    else
    {
        return;
    }

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

int my_il2cpp_init(const char* domain_name) 
{
    int result = ((il2cpp_init_fn)original_il2cpp_init)(domain_name);

    LOGI("IL2CPP is ready! Domain created. Starting dynamic hooks...");

    IL2CPP::InitIl2CppAPI();

    uintptr_t il2cpp_lib = get_base_addres("libil2cpp.so");

    SDK::Init(il2cpp_lib);
    InitAllMods();
    InitUnityInput();

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

                menu = std::make_unique<NativeMenu>(env, currentActivity);

                LOGI("Trying float button");
                floatBtn = std::make_unique<FloatButton>(env, currentActivity, [](){ LOGI("Alright, this shit is working"); menu->Show(); });
                LOGI("Created float button");

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
                menu->AddButton("Close Menu", []() { menu->Hide(); });
                
                menu->Hide();

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