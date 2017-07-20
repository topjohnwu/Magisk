package com.topjohnwu.magisk.utils;

import java.lang.ref.WeakReference;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

public class CallbackEvent<Result> {

    public boolean isTriggered = false;
    private Result result;
    private List<WeakReference<Listener<Result>>> listeners;

    public void register(Listener<Result> l) {
        if (listeners == null) {
            listeners = new LinkedList<>();
        }
        listeners.add(new WeakReference<>(l));
    }

    public void unRegister() {
        listeners = null;
    }

    public void unRegister(Listener<Result> l) {
        for (Iterator<WeakReference<Listener<Result>>> i = listeners.iterator(); i.hasNext();) {
            WeakReference<Listener<Result>> listener = i.next();
            if (listener.get() == null || listener.get() == l) {
                i.remove();
            }
        }
    }

    public void trigger() {
        trigger(null);
    }

    public void trigger(Result r) {
        result = r;
        isTriggered = true;
        if (listeners != null) {
            for (WeakReference<Listener<Result>> listener : listeners) {
                if (listener.get() != null)
                    listener.get().onTrigger(this);
            }
        }
    }

    public Result getResult() {
        return result;
    }

    public interface Listener<R> {
        void onTrigger(CallbackEvent<R> event);
    }
}
