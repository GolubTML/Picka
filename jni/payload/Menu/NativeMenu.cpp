#include "NativeMenu.h"
#include "log.h"

NativeMenu::NativeMenu(JNIEnv* jEnv, jobject context) : env(jEnv)
{
    jc.linearLayout = (jclass)env->NewGlobalRef(env->FindClass("android/widget/LinearLayout"));
    jc.windowManagerLP = (jclass)env->NewGlobalRef(env->FindClass("android/view/WindowManager$LayoutParams"));
    jc.linearLayoutLP = (jclass)env->NewGlobalRef(env->FindClass("android/widget/LinearLayout$LayoutParams"));
    jc.viewGroup = (jclass)env->NewGlobalRef(env->FindClass("android/view/ViewGroup"));
    jc.editText = (jclass)env->NewGlobalRef(env->FindClass("android/widget/EditText"));
    jc.view = (jclass)env->NewGlobalRef(env->FindClass("android/view/View"));
    jc.button = (jclass)env->NewGlobalRef(env->FindClass("android/widget/Button"));
    jc.context = (jclass)env->NewGlobalRef(env->FindClass("android/content/Context"));
    jc.imm = (jclass)env->NewGlobalRef(env->FindClass("android/view/inputmethod/InputMethodManager"));
    
    jmethodID llInit = env->GetMethodID(jc.linearLayout, "<init>", "(Landroid/content/Context;)V");
    rootLayout = env->NewGlobalRef(env->NewObject(jc.linearLayout, llInit, context));
    
    jmethodID setOrientation = env->GetMethodID(jc.linearLayout, "setOrientation", "(I)V");
    env->CallVoidMethod(rootLayout, setOrientation, 1);
    
    jmethodID setBG = env->GetMethodID(jc.linearLayout, "setBackgroundColor", "(I)V");
    env->CallVoidMethod(rootLayout, setBG, 0xDD2C2C2C);
    
    jmethodID lpInit = env->GetMethodID(jc.windowManagerLP, "<init>", "(IIIII)V");
    layoutParametrs = env->NewGlobalRef(env->NewObject(
        jc.windowManagerLP , lpInit,
        800, -2,
        2,
        8,
        -3
    ));

    jfieldID gravityField = env->GetFieldID(jc.windowManagerLP , "gravity", "I");
    env->SetIntField(layoutParametrs, gravityField, 0x11 /*CENTER*/);
    
    jmethodID getWM = env->GetMethodID(env->FindClass("android/content/Context"), "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    windowManager = env->NewGlobalRef(env->CallObjectMethod(context, getWM, env->NewStringUTF("window")));
    
    jc.windowManager = (jclass)env->NewGlobalRef(env->GetObjectClass(windowManager));
}

NativeMenu::~NativeMenu()
{
    if (rootLayout) env->DeleteGlobalRef(rootLayout);
    if (layoutParametrs) env->DeleteGlobalRef(layoutParametrs);
    if (windowManager) env->DeleteGlobalRef(windowManager);

    jc.Release(env);
}

void NativeMenu::AddButton(const std::string& text, std::function<void()> onClick)
{
    jobject button = createButton(text);

    MenuButton mBtn;
    mBtn.ref = env->NewGlobalRef(button);
    mBtn.fn = onClick;
    mBtn.isInput = false;
    mBtn.wasPressed = false;
    mBtn.isFocused = false;
    buttons.push_back(mBtn);

    jmethodID addView = env->GetMethodID(jc.viewGroup, "addView", "(Landroid/view/View;)V");
    env->CallVoidMethod(rootLayout, addView, button);
}

void NativeMenu::AddInputButton(const std::string& hint, const std::string& text, std::function<void(std::string)> onInput)
{
    jmethodID addView = env->GetMethodID(jc.viewGroup, "addView", "(Landroid/view/View;)V");

    jmethodID getCtx = env->GetMethodID(jc.view, "getContext", "()Landroid/content/Context;");
    jobject context = env->CallObjectMethod(rootLayout, getCtx);

    jmethodID llInit = env->GetMethodID(jc.linearLayout, "<init>", "(Landroid/content/Context;)V");
    jobject rowLayout = env->NewObject(jc.linearLayout, llInit, context);
    
    jmethodID setOrientation = env->GetMethodID(jc.linearLayout, "setOrientation", "(I)V");
    env->CallVoidMethod(rowLayout, setOrientation, 0);

    jobject editText = env->NewObject(jc.editText, env->GetMethodID(jc.editText, "<init>", "(Landroid/content/Context;)V"), context);
    
    /*jmethodID setSingleLine = env->GetMethodID(etClass, "setSingleLine", "(Z)V");
    env->CallVoidMethod(editText, setSingleLine, JNI_TRUE);*/

    jmethodID setImeOptions = env->GetMethodID(jc.editText, "setImeOptions", "(I)V");
    env->CallVoidMethod(editText, setImeOptions, 6);

    jmethodID setHint = env->GetMethodID(jc.editText, "setHint", "(Ljava/lang/CharSequence;)V");
    env->CallVoidMethod(editText, setHint, env->NewStringUTF(hint.c_str()));
    
    env->CallVoidMethod(editText, env->GetMethodID(jc.editText, "setTextColor", "(I)V"), 0xFFFFFFFF);

    jmethodID lpInit = env->GetMethodID(jc.linearLayoutLP, "<init>", "(IIF)V");
    jobject etParams = env->NewObject(jc.linearLayoutLP, lpInit, 0, -2, 1.0f); 
    env->CallVoidMethod(editText, env->GetMethodID(jc.editText, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V"), etParams);

    jobject button = createButton(text);

    env->CallVoidMethod(rowLayout, addView, editText);
    env->CallVoidMethod(rowLayout, addView, button);   
    env->CallVoidMethod(rootLayout, addView, rowLayout);

    MenuButton mBtn;
    mBtn.ref = env->NewGlobalRef(button);
    mBtn.inputRef = env->NewGlobalRef(editText);
    mBtn.fnInput = onInput;
    mBtn.isInput = true;
    mBtn.wasPressed = false;
    mBtn.isFocused = false;
    
    buttons.push_back(mBtn);
}

void NativeMenu::UpdateButtons()
{    
    static jmethodID isPressedID = env->GetMethodID(jc.view, "isPressed", "()Z");
    static jmethodID clearFocusID = env->GetMethodID(jc.view, "clearFocus", "()V");
    static jmethodID setTextID = env->GetMethodID(jc.editText, "setText", "(Ljava/lang/CharSequence;)V");

    for (auto& btn : buttons)
    {
        if (btn.isInput)
        {
            static jmethodID isFocusedID = env->GetMethodID(jc.view, "isFocused", "()Z");
            bool currentlyFocused = env->CallBooleanMethod(btn.inputRef, isFocusedID);

            if (currentlyFocused && !btn.isFocused) 
            {
                SetFocusable(true); 
                btn.isFocused = true;
            }
        }

        bool isCurrentlyPressed = env->CallBooleanMethod(btn.ref, isPressedID);

        if (isCurrentlyPressed && !btn.wasPressed)
        {
            if (btn.isInput && btn.fnInput)
            {
                std::string resultText = GetInputText(btn.inputRef); 
                btn.fnInput(resultText);

                // CloseKeyboard(btn.inputRef); i dont need this maybe
                SetFocusable(false);
                btn.isFocused = false;

                env->CallVoidMethod(btn.inputRef, clearFocusID);

                jstring emptyStr = env->NewStringUTF("");
                env->CallVoidMethod(btn.inputRef, setTextID, emptyStr);
                env->DeleteLocalRef(emptyStr);

            }
            else if (btn.fn) 
            {
                btn.fn(); 
            }
        }

        btn.wasPressed = isCurrentlyPressed;
    }
}

void NativeMenu::Show() 
{
    if (rootLayout && windowManager && layoutParametrs) 
    {
        jmethodID addView = env->GetMethodID(jc.windowManager, "addView", "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
        
        env->CallVoidMethod(windowManager, addView, rootLayout, layoutParametrs);
        LOGI("Menu Show called");
    }
}

void NativeMenu::Hide() {
    if (rootLayout && windowManager) 
    {
        jmethodID removeView = env->GetMethodID(jc.windowManager, "removeView", "(Landroid/view/View;)V");
        
        env->CallVoidMethod(windowManager, removeView, rootLayout);
        LOGI("Menu Hide called");
    }
}

void NativeMenu::SetFocusable(bool focusable)
{
    jfieldID flagsField = env->GetFieldID(jc.windowManagerLP, "flags", "I");

    int flags = env->GetIntField(layoutParametrs, flagsField);

    if (focusable)
        flags = (flags & ~8) | 32;
    else
        flags = (flags & ~32) | 8;

    env->SetIntField(layoutParametrs, flagsField, flags);

    jmethodID updateView = env->GetMethodID(jc.windowManager, "updateViewLayout", "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
    env->CallVoidMethod(windowManager, updateView, rootLayout, layoutParametrs);
}

jobject NativeMenu::createButton(const std::string& str)
{
    jmethodID getCtx = env->GetMethodID(jc.view, "getContext", "()Landroid/content/Context;");
    jobject context = env->CallObjectMethod(rootLayout, getCtx);

    jmethodID btnInit = env->GetMethodID(jc.button, "<init>", "(Landroid/content/Context;)V");
    jobject btn = env->NewObject(jc.button, btnInit, context);

    jmethodID setText = env->GetMethodID(jc.button, "setText", "(Ljava/lang/CharSequence;)V");
    env->CallVoidMethod(btn, setText, env->NewStringUTF(str.c_str()));

    jmethodID setAllCaps = env->GetMethodID(jc.button, "setAllCaps", "(Z)V");
    env->CallVoidMethod(btn, setAllCaps, JNI_FALSE);

    return btn;
}

void NativeMenu::CloseKeyboard(jobject view)
{
    jstring immService = env->NewStringUTF("input_method");
    jmethodID getSystemService = env->GetMethodID(jc.context, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    
    jmethodID getCtx = env->GetMethodID(jc.view, "getContext", "()Landroid/content/Context;");
    jobject context = env->CallObjectMethod(view, getCtx);
    jobject imm = env->CallObjectMethod(context, getSystemService, immService);

    jmethodID getWindowToken = env->GetMethodID(jc.view, "getWindowToken", "()Landroid/os/IBinder;");
    jobject token = env->CallObjectMethod(view, getWindowToken);

    jmethodID hideMethod = env->GetMethodID(jc.imm, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)V");
    
    env->CallVoidMethod(imm, hideMethod, token, 0);
}

std::string NativeMenu::GetInputText(jobject input)
{
    if (!input) return "";

    jmethodID getText = env->GetMethodID(jc.editText, "getText", "()Landroid/text/Editable;");
    jobject editable = env->CallObjectMethod(input, getText);
    jmethodID toString = env->GetMethodID(env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;"); // probobly crash here
    jstring jstr = (jstring)env->CallObjectMethod(editable, toString);

    const char* str = env->GetStringUTFChars(jstr, nullptr);
    std::string result(str);
    env->ReleaseStringUTFChars(jstr, str);

    return result;
}