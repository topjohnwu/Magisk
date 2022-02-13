package com.topjohnwu.magisk;

import com.topjohnwu.magisk.utils.DynamicClassLoader;

import java.io.File;
import java.util.Map;

// Wrap the actual classloader as we only want to resolve classname
// mapping when loading from platform (via LoadedApk.mClassLoader)
class InjectedClassLoader extends ClassLoader {

    InjectedClassLoader(File apk) {
        super(new DynamicClassLoader(apk));
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        String clz = DynLoad.componentMap.get(name);
        name = clz != null ? clz : name;
        return super.loadClass(name, resolve);
    }
}

class RedirectClassLoader extends ClassLoader {

    private final Map<String, Class<?>> mapping;

    RedirectClassLoader(Map<String, Class<?>> m) {
        super(RedirectClassLoader.class.getClassLoader());
        mapping = m;
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        Class<?> clz = mapping.get(name);
        return clz == null ? super.loadClass(name, resolve) : clz;
    }
}

class DelegateClassLoader extends ClassLoader {

    DelegateClassLoader() {
        super(null);
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        return DynLoad.loader.loadClass(name);
    }
}
