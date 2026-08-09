#include "chat_menu.h"
#include "../SDK/Il2Cpp/Il2CppAPI.h"
#include "../SDK/Il2Cpp/Il2CppResolver.h"
#include "../SDK/SDK.h"
#include "../log.h"
#include "../SDK/LuaAPI/core/lua_mod.h"
#include "../SDK/ModLoader.h"
#include <string>

size_t cached_text_offset = 0;

t_ProcessIncomingMessage orig_ProcessIncomingMessage = nullptr;
typedef int (*t_NewItem)(int, int, int, int, int, int, bool, int, bool, void*);

bool is_valid_ptr(void* p) 
{
    if (!p || (uintptr_t)p < 0x1000000) return false;
    
    return true;
}

int GetMyPlayerIndex() 
{
    static int (*get_myPlayer)() = nullptr;

    get_myPlayer = (int (*)())IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Main", "get_myPlayer", 0);
    
    return (get_myPlayer) ? get_myPlayer() : 0;
}

void* GetLocalPlayer()
{
    static void* main_klass = IL2CPP::Resolver::FindClass("Assembly-CSharp", "Terraria", "Main");
    if (!main_klass) return nullptr;

    static void* myPlayer_field = IL2CPP::Resolver::FindField(main_klass, "myPlayer");
    static void* player_field = IL2CPP::Resolver::FindStaticField(main_klass, "player");

    LOGI("Fields: %p, %p", myPlayer_field, player_field);
    void* static_data = IL2CPP::get_static_field_data(main_klass);
    
    int myPlayerIdx = *(int*)((uintptr_t)static_data + 0x424);

    void* playerArray = nullptr;
    IL2CPP::field_static_get_value(player_field, &playerArray);

    if (!playerArray || myPlayerIdx < 0) return nullptr;

    void** items = (void**)((uintptr_t)playerArray + 0x20); 
    return items[myPlayerIdx];
}

void my_ProcessIncomingMessage(void* instance, void* chatMessage, int clientID)
{
    LOGI("Chat Hook: instance=%p, msg=%p, ID=%d", instance, chatMessage, clientID);

    if (cached_text_offset == 0) 
    {
        void* klass = *(void**)chatMessage; 
        void* iter = nullptr;
        void* field;

        LOGI("Exploring fields of class: %p", klass);

        while ((field = IL2CPP::class_get_fields(klass, &iter))) 
        {
            const char* name = IL2CPP::field_get_name(field);
            size_t offset = IL2CPP::field_get_offset(field);
            LOGI("  Field: %s at offset 0x%lx", name, offset);

            if (std::string(name).find("Text") != std::string::npos) 
            {
                cached_text_offset = IL2CPP::field_get_offset(field);
                LOGI("!!! Found Text offset: 0x%lx", cached_text_offset);

                break;
            }
        }
    }

    if (cached_text_offset != 0)
    {
        void* il2cpp_str = *(void**)((uintptr_t)chatMessage + cached_text_offset);
        std::string cmd = IL2CPP::Resolver::GetString(il2cpp_str);

        if (!cmd.empty())
        {
            if (cmd == "!hi")
            {
                SDK::Chat("Hi from Picka!", { 0, 255, 0 });
                return;
            }

            if (cmd.find("!item ") == 0)
            {
                int itemID = std::stoi(cmd.substr(6));

                static void* method_ptr = IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Item", "NewItem", 9);

                if (method_ptr) 
                {
                    static auto NewItem = (t_NewItem)(*(void**)method_ptr);
                            
                    void* player = GetLocalPlayer();
                    if (player) 
                    {
                        static void* posField = IL2CPP::Resolver::FindField(*(void**)player, "position");

                        Vector2 pos = IL2CPP::Resolver::GetFieldValue<Vector2>(player, posField);
                        
                        LOGI("Player pos: %f, %f", pos.x, pos.y);
                        
                        NewItem((int)pos.x, (int)pos.y, 0, 0, itemID, 1, false, 0, false, nullptr);

                        SDK::Chat("Item spawned!", {255, 255, 0});
                    }
                }
            }

            if (cmd.find("!modlist") == 0) 
            { 
                ModLoader* loader = API::getModLoader();
                if (!loader)
                {
                    SDK::Chat("ModLoader not avaible!", {255, 0, 0});
                    return;
                }

                auto& mods = loader->getAllMods();

                if (mods.empty())
                {
                    SDK::Chat("No mods loaded", {255, 255, 0});
                    return;
                }

                SDK::Chat("Loaded mods:", {0, 255, 255});
                for (auto& mod : mods)
                {
                    std::string line = " -- " + mod.name + " v" + mod.version + " by " + mod.author;
                    SDK::Chat(line.c_str(), {255, 255, 255});
                }

                std::string totalMods = "Total mods: " + std::to_string(mods.size());
                SDK::Chat(totalMods.c_str(), {255, 255, 0});
            }
        }
    }

    
    orig_ProcessIncomingMessage(instance, chatMessage, clientID);
}