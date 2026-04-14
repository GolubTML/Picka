#include "chat_menu.h"
#include "../SDK/Il2CppAPI.h"
#include "../SDK/SDK.h"
#include "../log.h"
#include <string>

t_SendChat orig_SendChat = nullptr;

void my_SendChat(void* chatMessage)
{
    void* il2cpp_str = *(void**)((uintptr_t)chatMessage + 0x10);
    
    if (il2cpp_str && IL2CPP::string_to_utf8)
    {
        const char* c_str = IL2CPP::string_to_utf8(il2cpp_str);
        if (c_str) 
        {
            std::string command(c_str);
            LOGI("ChatHelper intercepted: %s", c_str);

            if (command.rfind("!hi", 0) == 0) 
            {
                SDK::Chat("Hello from ChatHelper!", { 0, 255, 0 });
                return; 
            }
            
            if (command.rfind("!god", 0) == 0)
            {
                SDK::Chat("God mode toggled!", { 255, 255, 0 });
                return;
            }
        }
    }
    
    orig_SendChat(chatMessage);
}