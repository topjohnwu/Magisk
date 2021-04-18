package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AppComponentFactory;
import android.app.Application;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentProvider;
import android.content.Intent;

@SuppressLint("NewApi")
public class DelegateComponentFactory extends AppComponentFactory {

    ClassLoader loader;
    AppComponentFactory receiver;

    public DelegateComponentFactory() {
        InjectAPK.componentFactory = this;
    }

    @Override
    public Application instantiateApplication(ClassLoader cl, String className) {
        if (loader == null) loader = cl;
        return new DelegateApplication();
    }

    @Override
    public Activity instantiateActivity(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateActivity(loader, className, intent);
        return create(className);
    }

    @Override
    public BroadcastReceiver instantiateReceiver(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateReceiver(loader, className, intent);
        return create(className);
    }

    @Override
    public Service instantiateService(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateService(loader, className, intent);
        return create(className);
    }

    @Override
    public ContentProvider instantiateProvider(ClassLoader cl, String className)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (receiver != null)
            return receiver.instantiateProvider(loader, className);
        return create(className);
    }

    private <T> T create(String name)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException{
        return (T) loader.loadClass(name).newInstance();
    }

}
