#pragma once

#include <jni.h>
#include <string>
#include <functional>
#include <vector>

struct JavaClasses
{
    jclass linearLayout;
    jclass windowManagerLP;
    jclass linearLayoutLP;
    jclass viewGroup;
    jclass editText;
    jclass view;
    jclass windowManager;
    jclass button;
    jclass context;
    jclass imm;

    void Release(JNIEnv* env)
    {
        env->DeleteGlobalRef(linearLayout);
        env->DeleteGlobalRef(windowManagerLP);
        env->DeleteGlobalRef(linearLayoutLP);
        env->DeleteGlobalRef(viewGroup);
        env->DeleteGlobalRef(editText);
        env->DeleteGlobalRef(view);
        env->DeleteGlobalRef(windowManager);
        env->DeleteGlobalRef(button);
        env->DeleteGlobalRef(context);
        env->DeleteGlobalRef(imm);
    }
};

struct MenuButton
{
    /*
    So, this structure not only allows us to make a button, but else an input button
    But, maybe i need write 2 seperate structures, like MenuButton and InputButton
    For now, it will be in one structure
    */

    jobject ref;
    jobject inputRef; // referens to EditText

    std::function<void()> fn;
    std::function<void(std::string str)> fnInput; 

    bool isInput;
    bool wasPressed;
    bool isFocused;
};

class NativeMenu
{
private:
    std::vector<MenuButton> buttons;

    jobject rootLayout;
    jobject windowManager;
    jobject layoutParametrs;
    JNIEnv* env;
    
    JavaClasses jc; // more bad ideas

    jobject createButton(const std::string& str);
    void applyStyle(jobject view);

    void CloseKeyboard(jobject view);

    std::string GetInputText(jobject input);

public:
    NativeMenu(JNIEnv* jEnv, jobject context);
    ~NativeMenu();

    void AddButton(const std::string& text, std::function<void()> onClick);
    void AddInputButton(const std::string& hint, const std::string& text, std::function<void(std::string)> onInput);
    void UpdateButtons();

    void Show();
    void Hide();

    void SetFocusable(bool focused);
};