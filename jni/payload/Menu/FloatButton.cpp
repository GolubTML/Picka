#include "FloatButton.h"
#include "log.h"

FloatButton::FloatButton(JNIEnv* _env,jobject currActivity, std::function<void()> _onClick) 
    : env(_env), onClick(_onClick)
{
    btnClass = (jclass)env->NewGlobalRef(env->FindClass("android/widget/Button"));
    ctxClass = (jclass)env->NewGlobalRef(env->FindClass("android/content/Context"));
    lpClass = (jclass)env->NewGlobalRef(env->FindClass("android/view/WindowManager$LayoutParams"));


    if (!btnClass) LOGI("ERR: No button class");
    if (!ctxClass) LOGI("ERR: No context class");

    LOGI("Fount button and context class");

    jmethodID btnInit = env->GetMethodID(btnClass, "<init>", "(Landroid/content/Context;)V");
    LOGI("local button created?");
    jobject localBtn = env->NewObject(btnClass, btnInit, currActivity); // crash. Why it's currently here?

    if (!localBtn) 
    {
        LOGI("localBtn is null!");
        return;
    }

    LOGI("local button now created!");
    mButton = env->NewGlobalRef(localBtn);
    LOGI("Created button?");

    jmethodID setText = env->GetMethodID(btnClass, "setText", "(Ljava/lang/CharSequence;)V");
    env->CallVoidMethod(mButton, setText, env->NewStringUTF("M"));

    jmethodID setAlpha = env->GetMethodID(btnClass, "setAlpha", "(F)V");
    env->CallVoidMethod(mButton, setAlpha, 0.7f);

    LOGI("Button apply customization");

    jmethodID lpInit = env->GetMethodID(lpClass, "<init>", "(IIIII)V");

    mParams = env->NewGlobalRef(env->NewObject(lpClass, lpInit,
        50, 50,                      
        2,                              
        8 | 16 | 512,                           
        -3                        
    ));

    LOGI("Created Button!");

    jfieldID gravityField = env->GetFieldID(lpClass, "gravity", "I");
    env->SetIntField(mParams, gravityField, 0x33);

    jfieldID xField = env->GetFieldID(lpClass, "x", "I");
    jfieldID yField = env->GetFieldID(lpClass, "y", "I");
    env->SetIntField(mParams, xField, 100);
    env->SetIntField(mParams, yField, 100);

    LOGI("Button now on the left side");

    jmethodID getSystemServ = env->GetMethodID(ctxClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject wm = env->CallObjectMethod(currActivity, getSystemServ, env->NewStringUTF("window"));
    mWindowManager = env->NewGlobalRef(wm);

    LOGI("Window manager isn't null");

    jmethodID addView = env->GetMethodID(env->GetObjectClass(mWindowManager), "addView", "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
    env->CallVoidMethod(mWindowManager, addView, mButton, mParams);

    updateViewLayoutID = env->GetMethodID(
        env->GetObjectClass(mWindowManager), 
        "updateViewLayout", 
        "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V"
    );
    
    LOGI("Everything is done!");
}

FloatButton::~FloatButton()
{
    if (mWindowManager && mButton) 
    {
        jmethodID removeView = env->GetMethodID(
            env->GetObjectClass(mWindowManager), 
            "removeView", 
            "(Landroid/view/View;)V"
        );

        env->CallVoidMethod(mWindowManager, removeView, mButton);
    }

    if (btnClass) env->DeleteGlobalRef(btnClass);
    if (ctxClass) env->DeleteGlobalRef(ctxClass);

    if (mButton) env->DeleteGlobalRef(mButton);
    if (mWindowManager) env->DeleteGlobalRef(mWindowManager);
    if (mParams) env->DeleteGlobalRef(mParams);
}

void FloatButton::OnTouch(int action, int rawX, int rawY)
{
    jfieldID xField = env->GetFieldID(lpClass, "x", "I");
    jfieldID yField = env->GetFieldID(lpClass, "y", "I");

    switch (action)
    {
    case 0: // Action Down
        initialX = env->GetIntField(mParams, xField);
        initialY = env->GetIntField(mParams, yField);

        LOGI("Touch: %d, %d | Button: %d, %d", rawX, rawY, initialX, initialY);

        if (rawX >= initialX && rawX <= (initialX + 75) && 
            rawY >= initialY && rawY <= (initialY + 75)) 
        {
            activeTracking = true;  
            initialTouchX = rawX;
            initialTouchY = rawY;
            isMoving = false;
            LOGI("Button GRABBED! Real hit!");
        }
        else 
        {
            activeTracking = false; 
        }
        break;
        
    case 2: // Action Move
        if (!activeTracking) return;    

        {
            int dx = rawX - initialTouchX;
            int dy = rawY - initialTouchY;

            if (abs(dx) > 5 || abs(dy) > 5) 
            {
                isMoving = true;
                env->SetIntField(mParams, xField, initialX + dx);
                env->SetIntField(mParams, yField, initialY + dy);

                env->CallVoidMethod(mWindowManager, updateViewLayoutID, mButton, mParams);
            }
        }
        break;

    case 1: // Action Up
        if (!activeTracking) return;

        if (!isMoving) 
            if (onClick) onClick();

        activeTracking = false;
        break;

    default:
        break;
    }
}