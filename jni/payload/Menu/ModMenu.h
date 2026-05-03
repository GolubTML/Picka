#pragma once

#include "NativeMenu.h"
#include "FloatButton.h"
#include "../SDK/SDK.h"
#include <memory>

namespace Menu
{
    class ModMenu
    {
    private:
        // maybe bad place for button func

        void SetGhostPlayer();
        void SpawItem(int itemID);

    public:
        std::unique_ptr<NativeMenu> menu;
        std::unique_ptr<FloatButton> floatBtn;

        ModMenu(JNIEnv* _env, jobject activity);

        void OnUpdate();
    };

    extern std::unique_ptr<ModMenu> instance;

    typedef void (*t_Main_Update)(void* gameTime);
    typedef Vector3 (*t_get_mousePosition)();
    typedef bool (*t_GetMouseButton)(int);
    typedef int (*t_get_ScreenValue)();
    
    extern t_Main_Update orig_Main_Update;
    extern t_get_mousePosition get_mousePosition;
    extern t_GetMouseButton get_mouseButton;
    extern t_get_ScreenValue get_screenWidth;
    extern t_get_ScreenValue get_screenHeight;

    void InitUnityInput();
    void my_Update(void* gameTime);
}