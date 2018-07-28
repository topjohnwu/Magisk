package com.topjohnwu.snet;

import android.app.Activity;

import java.lang.reflect.Proxy;

public class Snet {
    public static Object newHelper(Class<?> clazz, String dexPath, Activity activity, Object cb) {
        ModdedGPSUtil.dexPath = dexPath;
        return Proxy.newProxyInstance(SafetyNetHelper.class.getClassLoader(),
                new Class[] { clazz }, new SafetyNetHelper(activity, cb));
    }
}
