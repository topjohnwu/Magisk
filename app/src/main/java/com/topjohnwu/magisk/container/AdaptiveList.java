package com.topjohnwu.magisk.container;

import java.util.ArrayList;

public abstract class AdaptiveList<E> extends ArrayList<E> {

    private Runnable callback;

    public abstract void updateView();

    public void setCallback(Runnable cb) {
        callback = cb;
    }

    public synchronized boolean add(E e) {
        boolean ret = super.add(e);
        if (ret) {
            if (callback == null) {
                updateView();
            } else {
                callback.run();
            }
        }
        return ret;
    }
}
