#include "SDK.h"
#include "Il2CppAPI.h"
#include "../log.h"
#include <dlfcn.h>

void SDK::Init(uintptr_t base)
{
    void* newText_fn = IL2CPP::FindMethod("Assembly-CSharp", "Terraria", "Main", "NewText", 4);

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