#include "SDK.h"
#include "Il2Cpp/Il2CppAPI.h"
#include "Il2Cpp/Il2CppResolver.h"
#include "../log.h"
#include <dlfcn.h>

void SDK::Init(uintptr_t base)
{
    void* newText_fn = IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Main", "NewText", 4);

    if (newText_fn) 
    {
        NewText = (decltype(NewText))(*(void**)newText_fn);
        LOGI("SDK: NewText initialized at %p", NewText);
    }
}

void SDK::Chat(const char* message, Color color)
{
    if (NewText && IL2CPP::new_string)
        NewText(IL2CPP::new_string(message), color.r, color.g, color.b);
}

void* SDK::GetLocalPlayer()
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