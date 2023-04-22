package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AppComponentFactory;
import android.app.Application;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentProvider;
import android.content.Intent;
import android.content.pm.ApplicationInfo;

import com.topjohnwu.magisk.dummy.DummyProvider;
import com.topjohnwu.magisk.dummy.DummyReceiver;
import com.topjohnwu.magisk.dummy.DummyService;

@SuppressLint("NewApi")
public class DelegateComponentFactory extends AppComponentFactory {

    AppComponentFactory receiver;

    public DelegateComponentFactory() {
        DynLoad.componentFactory = this;
    }

    @Override
    public ClassLoader instantiateClassLoader(ClassLoader cl, ApplicationInfo info) {
        return new DelegateClassLoader();
    }

    @Override
    public Application instantiateApplication(ClassLoader cl, String className) {
        return new DelegateApplication();
    }

    @Override
    public Activity instantiateActivity(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateActivity(DynLoad.activeClassLoader, className, intent);
        return create(className, DownloadActivity.class);
    }

    @Override
    public BroadcastReceiver instantiateReceiver(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateReceiver(DynLoad.activeClassLoader, className, intent);
        return create(className, DummyReceiver.class);
    }

    @Override
    public Service instantiateService(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateService(DynLoad.activeClassLoader, className, intent);
        return create(className, DummyService.class);
    }

    @Override
    public ContentProvider instantiateProvider(ClassLoader cl, String className)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateProvider(DynLoad.activeClassLoader, className);
        return create(className, DummyProvider.class);
    }

    private <T> T create(String name, Class<T> fallback)
            throws IllegalAccessException, InstantiationException {
        try {
            // noinspection unchecked
            return (T) DynLoad.activeClassLoader.loadClass(name).newInstance();
        } catch (ClassNotFoundException e) {
            return fallback.newInstance();
        }
    }

}
