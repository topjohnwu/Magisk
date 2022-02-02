package com.topjohnwu.magisk;

import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;

import java.lang.reflect.Constructor;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public class DelegateRootService extends ContextWrapper {

    public DelegateRootService() {
        super(null);
    }

    @Override
    protected void attachBaseContext(Context base) {
        if (DynLoad.createApp(base) == null)
            return;

        // Create the actual RootService and call its attachBaseContext
        try {
            Constructor<?> ctor = DynLoad.apkData.getRootService().getConstructor(Object.class);
            ctor.setAccessible(true);
            Object service = ctor.newInstance(this);
            DynLoad.attachContext(service, base);
        } catch (Exception e) {
            Log.e(DelegateRootService.class.getSimpleName(), "", e);
        }
    }
}
