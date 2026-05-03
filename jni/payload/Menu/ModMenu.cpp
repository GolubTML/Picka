#include "ModMenu.h"
#include "SDK/Il2Cpp/Il2CppResolver.h"
#include "log.h"

namespace Menu
{
    std::unique_ptr<ModMenu> instance = nullptr;

    t_Main_Update orig_Main_Update = nullptr;
    t_get_mousePosition get_mousePosition = nullptr;
    t_GetMouseButton get_mouseButton = nullptr;
    t_get_ScreenValue get_screenWidth = nullptr;
    t_get_ScreenValue get_screenHeight = nullptr;

    typedef int (*t_NewItem)(int, int, int, int, int, int, bool, int, bool, void*);

    void ModMenu::SetGhostPlayer()
    {
        void* player = SDK::GetLocalPlayer();
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

    void ModMenu::SpawItem(int itemID)
    {
        void* player = SDK::GetLocalPlayer();
        if (!player) return;

        static void* method_ptr = IL2CPP::Resolver::FindMethod("Assembly-CSharp", "Terraria", "Item", "NewItem", 9);
        if (!method_ptr) return;

        auto NewItem = (t_NewItem)(*(void**)method_ptr);

        static void* posField = IL2CPP::Resolver::FindField(*(void**)player, "position");
        Vector2 pos = IL2CPP::Resolver::GetFieldValue<Vector2>(player, posField);

        NewItem((int)pos.x, (int)pos.y, 0, 0, itemID, 1, false, 0, false, nullptr);
    }

    ModMenu::ModMenu(JNIEnv* _env, jobject activity)
    {
        menu = std::make_unique<NativeMenu>(_env, activity);
        floatBtn = std::make_unique<FloatButton>(_env, activity, [this](){  });

        menu->AddButton("Ghost Mode", [this]() { SetGhostPlayer(); });
        menu->AddInputButton("Type Item ID", "Spawn", [this](std::string str) 
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
        menu->AddButton("Close Menu", [this]() { this->menu->Hide(); });

        menu->Hide();
    }

    void ModMenu::OnUpdate()
    {
        if (menu)
            menu->UpdateButtons();
    }

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

        if (instance)
            instance->OnUpdate();
    }
}