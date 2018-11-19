package com.topjohnwu.snet;

import android.app.Activity;

import java.lang.reflect.Proxy;

public class Snet {
    static String dexPath;

    public static Object newHelper(Class<?> interfaceClass, String dexPath, Activity activity, Object cb) {
        Snet.dexPath = dexPath;
        return Proxy.newProxyInstance(SafetyNetHelper.class.getClassLoader(),
                new Class[] { interfaceClass }, new SafetyNetHelper(activity, cb));
    }
}
