package com.topjohnwu.magisk.utils;

import android.os.Process;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;

import dalvik.system.BaseDexClassLoader;

public class DynamicClassLoader extends BaseDexClassLoader {

    public DynamicClassLoader(File apk) {
        this(apk, DynamicClassLoader.class.getClassLoader());
    }

    public DynamicClassLoader(File apk, ClassLoader parent) {
        // Set optimizedDirectory to null for RootService to bypass DexFile's security checks
        super(apk.getPath(), Process.myUid() == 0 ? null : apk.getParentFile(), null, parent);
    }

    @Override
    protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
        // First check if already loaded
        Class<?> cls = findLoadedClass(name);
        if (cls != null)
            return cls;

        try {
            // Then check boot classpath
            return getSystemClassLoader().loadClass(name);
        } catch (ClassNotFoundException ignored) {
            try {
                // Next try current dex
                return findClass(name);
            } catch (ClassNotFoundException fromSuper) {
                try {
                    // Finally try parent
                    return getParent().loadClass(name);
                } catch (ClassNotFoundException e) {
                    throw fromSuper;
                }
            }
        }
    }

    @Override
    public URL getResource(String name) {
        URL resource = getSystemClassLoader().getResource(name);
        if (resource != null)
            return resource;
        resource = findResource(name);
        if (resource != null)
            return resource;
        resource = getParent().getResource(name);
        return resource;
    }

    @Override
    public Enumeration<URL> getResources(String name) throws IOException {
        return new CompoundEnumeration<>(getSystemClassLoader().getResources(name),
                findResources(name), getParent().getResources(name));
    }
}
