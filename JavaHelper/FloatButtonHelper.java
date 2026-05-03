package com.picka.tools;

import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

public class FloatButtonHelper implements View.OnTouchListener 
{
    public static native void onClickNative();

    private int initialX;
    private int initialY;
    private float initialTouchX;
    private float initialTouchY;
    private WindowManager wm;
    private WindowManager.LayoutParams params;
    private View view;

    public FloatButtonHelper(View view, WindowManager wm, WindowManager.LayoutParams params)
    {
        this.view = view;
        this.wm = wm;
        this.params = params;
    }

    public static void initTouchListener(final View view, final WindowManager wm, final WindowManager.LayoutParams params) 
    {
        view.setOnTouchListener(new FloatButtonHelper(view, wm, params));
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) 
    {
        switch (event.getAction()) 
        {
            case MotionEvent.ACTION_DOWN:
                initialX = params.x;
                initialY = params.y;
                initialTouchX = event.getRawX();
                initialTouchY = event.getRawY();
                return true;

            case MotionEvent.ACTION_MOVE:
                params.x = initialX + (int) (event.getRawX() - initialTouchX);
                params.y = initialY + (int) (event.getRawY() - initialTouchY);
                wm.updateViewLayout(view, params);
                return true;

            case MotionEvent.ACTION_UP:
                float diffX = Math.abs(event.getRawX() - initialTouchX);
                float diffY = Math.abs(event.getRawY() - initialTouchY);
                if (diffX < 10 && diffY < 10) 
                {
                    try {
                        onClickNative(); 
                    } catch (UnsatisfiedLinkError e) {
                        android.util.Log.e("Payload", "JNI Error: " + e.getMessage());
                    }

                }
                return true;
        }
        return false;
    }
}