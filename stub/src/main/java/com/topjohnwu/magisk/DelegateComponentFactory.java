package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AppComponentFactory;
import android.app.Application;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentProvider;
import android.content.Intent;

import com.topjohnwu.magisk.dummy.DummyActivity;
import com.topjohnwu.magisk.dummy.DummyProvider;
import com.topjohnwu.magisk.dummy.DummyReceiver;
import com.topjohnwu.magisk.dummy.DummyService;
import com.topjohnwu.magisk.obfuscate.Mapping;

@SuppressLint("NewApi")
public class DelegateComponentFactory extends AppComponentFactory {

    ClassLoader loader;
    AppComponentFactory delegate;

    @Override
    public Application instantiateApplication(ClassLoader cl, String className) {
        if (loader == null) loader = cl;
        return new DelegateApplication(this);
    }

    @Override
    public Activity instantiateActivity(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (delegate != null)
            return delegate.instantiateActivity(loader, Mapping.get(className), intent);
        return create(className, DummyActivity.class);
    }

    @Override
    public BroadcastReceiver instantiateReceiver(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (delegate != null)
            return delegate.instantiateReceiver(loader, Mapping.get(className), intent);
        return create(className, DummyReceiver.class);
    }

    @Override
    public Service instantiateService(ClassLoader cl, String className, Intent intent)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (delegate != null)
            return delegate.instantiateService(loader, Mapping.get(className), intent);
        return create(className, DummyService.class);
    }

    @Override
    public ContentProvider instantiateProvider(ClassLoader cl, String className)
            throws ClassNotFoundException, IllegalAccessException, InstantiationException {
        if (loader == null) loader = cl;
        if (delegate != null)
            return delegate.instantiateProvider(loader, Mapping.get(className));
        return create(className, DummyProvider.class);
    }

    /**
     * Create the class or dummy implementation if creation failed
     */
    private <T> T create(String name, Class<? extends T> dummy)
            throws InstantiationException, IllegalAccessException {
        try {
            return (T) loader.loadClass(name).newInstance();
        } catch (IllegalAccessException | InstantiationException | ClassNotFoundException ignored) {
            return dummy.newInstance();
        }
    }
}
