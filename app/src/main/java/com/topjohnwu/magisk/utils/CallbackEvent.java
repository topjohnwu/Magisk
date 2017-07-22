package com.topjohnwu.magisk.utils;

import java.lang.ref.WeakReference;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

public class CallbackEvent {

    public boolean isTriggered = false;
    private List<WeakReference<Listener>> listeners;

    public void register(Listener l) {
        if (listeners == null) {
            listeners = new LinkedList<>();
        }
        listeners.add(new WeakReference<>(l));
    }

    public void unregister() {
        listeners = null;
    }

    public void unregister(Listener l) {
        for (Iterator<WeakReference<Listener>> i = listeners.iterator(); i.hasNext();) {
            WeakReference<Listener> listener = i.next();
            if (listener.get() == null || listener.get() == l) {
                i.remove();
            }
        }
    }

    public void trigger() {
        trigger(true);
    }

    public void trigger(boolean b) {
        isTriggered = b;
        if (listeners != null) {
            for (WeakReference<Listener> listener : listeners) {
                if (listener.get() != null)
                    listener.get().onTrigger(this);
            }
        }
    }


    public interface Listener {
        default void registerEvents() {
            for (CallbackEvent event : getRegisterEvents()) {
                if (event.isTriggered) {
                    onTrigger(event);
                }
                event.register(this);
            }
        }
        default void unregisterEvents() {
            for (CallbackEvent event : getRegisterEvents()) {
                event.unregister(this);
            }
        }
        default void onTrigger() {
            onTrigger(null);
        }
        void onTrigger(CallbackEvent event);
        CallbackEvent[] getRegisterEvents();
    }
}
