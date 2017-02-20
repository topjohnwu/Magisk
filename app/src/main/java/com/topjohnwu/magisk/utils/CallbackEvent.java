package com.topjohnwu.magisk.utils;

import java.util.HashSet;
import java.util.Set;

public class CallbackEvent<Result> {

    public boolean isTriggered = false;
    private Result result;
    private Set<Listener<Result>> listeners;

    public void register(Listener<Result> l) {
        if (listeners == null) {
            listeners = new HashSet<>();
        }
        listeners.add(l);
    }

    public void unRegister() {
        listeners = null;
    }

    public void unRegister(Listener<Result> l) {
        if (listeners != null) {
            listeners.remove(l);
        }
    }

    public void trigger() {
        trigger(null);
    }

    public void trigger(Result r) {
        result = r;
        isTriggered = true;
        if (listeners != null) {
            for (Listener<Result> listener : listeners) {
                listener.onTrigger(this);
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
