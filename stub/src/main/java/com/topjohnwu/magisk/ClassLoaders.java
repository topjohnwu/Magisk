package com.topjohnwu.magisk;

import com.topjohnwu.magisk.utils.DynamicClassLoader;

import java.io.File;

class InjectedClassLoader extends DynamicClassLoader {

    InjectedClassLoader(File apk) {
        super(apk,
                /* Use the base classloader as we do not want stub
                 * APK classes accessible from the main app */
                Object.class.getClassLoader());
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        return super.loadClass(Mapping.get(name), resolve);
    }
}

class RedirectClassLoader extends ClassLoader {

    RedirectClassLoader() {
        super(RedirectClassLoader.class.getClassLoader());
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        Class<?> clz = Mapping.internalMap.get(name);
        return clz == null ? super.loadClass(name, resolve) : clz;
    }
}
