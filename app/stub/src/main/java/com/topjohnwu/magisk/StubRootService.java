package com.topjohnwu.magisk;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import java.io.File;
import java.lang.reflect.Constructor;

public class StubRootService extends ContextWrapper {

    public StubRootService() {
        super(null);
    }

    @Override
    protected void attachBaseContext(Context base) {
        ClassLoader loader = DynLoad.loadApk(base);
        if (loader == null)
            return;

        try {
            // Create application to get the real root service class
            var data = DynLoad.createApkData();
            File apk = StubApk.current(base);
            PackageManager pm = base.getPackageManager();
            PackageInfo pkgInfo = pm.getPackageArchiveInfo(apk.getPath(), 0);
            loader.loadClass(pkgInfo.applicationInfo.className)
                    .getConstructor(Object.class)
                    .newInstance(data.getObject());

            // Create the actual RootService and call its attachBaseContext
            Constructor<?> ctor = data.getRootService().getConstructor(Object.class);
            ctor.setAccessible(true);
            Object service = ctor.newInstance(this);
            DynLoad.attachContext(service, base);
        } catch (Exception e) {
            Log.e(StubRootService.class.getSimpleName(), "", e);
        }
    }
}
