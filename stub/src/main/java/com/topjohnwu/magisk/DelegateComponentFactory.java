package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AppComponentFactory;
import android.app.Application;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentProvider;
import android.content.Intent;

import com.topjohnwu.magisk.dummy.DummyProvider;
import com.topjohnwu.magisk.dummy.DummyReceiver;
import com.topjohnwu.magisk.dummy.DummyService;

@SuppressLint("NewApi")
public class DelegateComponentFactory extends AppComponentFactory {

    ClassLoader loader;
    AppComponentFactory delegate;

    interface DummyFactory<T> {
        T create();
    }

    public DelegateComponentFactory() {
        InjectAPK.factory = this;
    }

    @Override
    public Application instantiateApplication(ClassLoader cl, String className) {
        if (loader == null) loader = cl;
        return new DelegateApplication();
    }

    @Override
    public Activity instantiateActivity(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (delegate != null)
            return delegate.instantiateActivity(loader, className, intent);
        return create(className, DownloadActivity::new);
    }

    @Override
    public BroadcastReceiver instantiateReceiver(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (delegate != null)
            return delegate.instantiateReceiver(loader, className, intent);
        return create(className, DummyReceiver::new);
    }

    @Override
    public Service instantiateService(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (delegate != null)
            return delegate.instantiateService(loader, className, intent);
        return create(className, DummyService::new);
    }

    @Override
    public ContentProvider instantiateProvider(ClassLoader cl, String className)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (loader == null) loader = cl;
        if (delegate != null)
            return delegate.instantiateProvider(loader, className);
        return create(className, DummyProvider::new);
    }

    /**
     * Create the class or dummy implementation if creation failed
     */
    private <T> T create(String name, DummyFactory<T> factory) {
        try {
            return (T) loader.loadClass(name).newInstance();
        } catch (Exception ignored) {
            return factory.create();
        }
    }
}
