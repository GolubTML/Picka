#pragma once

#include <jni.h>
#include <functional>

/*
    Maybe, it's bad idea to make a seperate class for just a button
    But, this button uses different methods and classes, not like in NativeMenu
*/

class FloatButton
{
private:
    JNIEnv* env;

    jclass btnClass;
    jclass ctxClass;
    jclass lpClass;

    jobject mButton;
    jobject mWindowManager;
    jobject mParams;

    jmethodID updateViewLayoutID;

    std::function<void()> onClick;

    int initialX, initialY;
    int initialTouchX, initialTouchY;

    bool isMoving = false;
    bool activeTracking = false;

public:
    FloatButton(JNIEnv* _env, jobject currActivity, std::function<void()> _onClick);
    ~FloatButton();

    void OnTouch(int action, int rawX, int rawY);
};