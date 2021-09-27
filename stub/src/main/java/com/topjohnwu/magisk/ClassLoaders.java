package com.topjohnwu.magisk;

import com.topjohnwu.magisk.utils.DynamicClassLoader;

import java.io.File;

// Wrap the actual classloader as we only want to resolve classname
// mapping when loading from platform (via LoadedApk.mClassLoader)
class InjectedClassLoader extends ClassLoader {

    InjectedClassLoader(File apk) {
        super(new DynamicClassLoader(apk));
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
