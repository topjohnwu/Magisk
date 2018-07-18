package com.topjohnwu.magisk.components;

import android.os.Handler;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.util.Locale;

public abstract class Application extends android.app.Application {

    public static WeakReference<Application> weakSelf;
    public static Locale locale;
    public static Locale defaultLocale;

    private static Handler mHandler = new Handler();

    public Application() {
        weakSelf = new WeakReference<>(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        locale = defaultLocale = Locale.getDefault();
    }

    public static void toast(CharSequence msg, int duration) {
        mHandler.post(() -> Toast.makeText(weakSelf.get(), msg, duration).show());
    }

    public static void toast(int resId, int duration) {
        mHandler.post(() -> Toast.makeText(weakSelf.get(), resId, duration).show());
    }
}
