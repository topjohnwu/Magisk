package com.topjohnwu.magisk;

import android.app.job.JobService;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.ServiceInfo;

import com.topjohnwu.magisk.dummy.DummyProvider;
import com.topjohnwu.magisk.dummy.DummyReceiver;
import com.topjohnwu.magisk.dummy.DummyService;
import com.topjohnwu.magisk.utils.DynamicClassLoader;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

// Wrap the actual classloader as we only want to resolve classname
// mapping when loading from platform (via LoadedApk.mClassLoader)
class AppClassLoader extends ClassLoader {
    final Map<String, String> mapping = new HashMap<>();

    AppClassLoader(File apk) {
        super(new DynamicClassLoader(apk));
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        String clz = mapping.get(name);
        name = clz != null ? clz : name;
        return super.loadClass(name, resolve);
    }

    void updateComponentMap(PackageInfo stub, PackageInfo app) {
        {
            var src = stub.activities;
            var dest = app.activities;

            final ActivityInfo sa;
            final ActivityInfo da;
            final ActivityInfo sb;
            final ActivityInfo db;
            if (src[0].exported) {
                sa = src[0];
                sb = src[1];
            } else {
                sa = src[1];
                sb = src[0];
            }
            if (dest[0].exported) {
                da = dest[0];
                db = dest[1];
            } else {
                da = dest[1];
                db = dest[0];
            }
            mapping.put(sa.name, da.name);
            mapping.put(sb.name, db.name);
        }

        {
            var src = stub.services;
            var dest = app.services;

            final ServiceInfo sa;
            final ServiceInfo da;
            final ServiceInfo sb;
            final ServiceInfo db;
            if (JobService.PERMISSION_BIND.equals(src[0].permission)) {
                sa = src[0];
                sb = src[1];
            } else {
                sa = src[1];
                sb = src[0];
            }
            if (JobService.PERMISSION_BIND.equals(dest[0].permission)) {
                da = dest[0];
                db = dest[1];
            } else {
                da = dest[1];
                db = dest[0];
            }
            mapping.put(sa.name, da.name);
            mapping.put(sb.name, db.name);
        }

        {
            var src = stub.receivers;
            var dest = app.receivers;
            mapping.put(src[0].name, dest[0].name);
        }

        {
            var src = stub.providers;
            var dest = app.providers;
            mapping.put(src[0].name, dest[0].name);
        }
    }
}

class StubClassLoader extends ClassLoader {

    private final Map<String, Class<?>> mapping = new HashMap<>();

    StubClassLoader(PackageInfo info) {
        super(StubClassLoader.class.getClassLoader());
        for (var c : info.activities) {
            mapping.put(c.name, DownloadActivity.class);
        }
        for (var c : info.services) {
            mapping.put(c.name, DummyService.class);
        }
        for (var c : info.providers) {
            mapping.put(c.name, DummyProvider.class);
        }
        for (var c : info.receivers) {
            mapping.put(c.name, DummyReceiver.class);
        }
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        Class<?> clz = mapping.get(name);
        return clz == null ? super.loadClass(name, resolve) : clz;
    }
}

class DelegateClassLoader extends ClassLoader {

    DelegateClassLoader() {
        super();
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        return DynLoad.activeClassLoader.loadClass(name);
    }
}
