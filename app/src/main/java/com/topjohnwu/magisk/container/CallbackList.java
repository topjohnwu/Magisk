package com.topjohnwu.magisk.container;

import android.os.Handler;

import java.util.ArrayList;

public abstract class CallbackList<E> extends ArrayList<E> {

    private Handler handler;

    protected CallbackList() {
        handler = new Handler();
    }

    public abstract void onAddElement(E e);

    public synchronized boolean add(E e) {
        boolean ret = super.add(e);
        handler.post(() -> onAddElement(e));
        return ret;
    }
}
