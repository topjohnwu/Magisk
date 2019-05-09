package com.topjohnwu.magisk.utils;

import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Set;

import androidx.annotation.IntDef;
import androidx.collection.ArraySet;

@Deprecated
public class Event {

    @Deprecated
    public static final int LOCALE_FETCH_DONE = 4;

    @IntDef(LOCALE_FETCH_DONE)
    @Retention(RetentionPolicy.SOURCE)
    public @interface EventID {}

    // We will not dynamically add topics, so use arrays instead of hash tables
    private static final Store[] eventList = new Store[5];

    @Deprecated
    public static void register(Listener listener, @EventID int... events) {
        for (int event : events) {
            if (eventList[event] == null)
                eventList[event] = new Store();
            eventList[event].listeners.add(listener);
            if (eventList[event].triggered) {
                listener.onEvent(event);
            }
        }
    }

    @Deprecated
    public static void register(AutoListener listener) {
        register(listener, listener.getListeningEvents());
    }

    @Deprecated
    public static void unregister(Listener listener, @EventID int... events) {
        for (int event : events) {
            if (eventList[event] == null)
                continue;
            eventList[event].listeners.remove(listener);
        }
    }

    @Deprecated
    public static void unregister(AutoListener listener) {
        unregister(listener, listener.getListeningEvents());
    }

    @Deprecated
    public static void trigger(@EventID int event) {
        trigger(true, event, null);
    }

    @Deprecated
    public static void trigger(@EventID int event, Object result) {
        trigger(true, event, result);
    }

    @Deprecated
    public static void trigger(boolean perm, @EventID int event) {
        trigger(perm, event, null);
    }

    @Deprecated
    public static void trigger(boolean perm, @EventID int event, Object result) {
        if (eventList[event] == null)
            eventList[event] = new Store();
        if (perm) {
            eventList[event].result = result;
            eventList[event].triggered = true;
        }
        for (Listener sub : eventList[event].listeners) {
            UiThreadHandler.run(() -> sub.onEvent(event));
        }
    }

    @Deprecated
    public static void reset(@EventID int event) {
        if (eventList[event] == null)
            return;
        eventList[event].triggered = false;
        eventList[event].result = null;
    }

    @Deprecated
    public static void reset(AutoListener listener) {
        for (int event : listener.getListeningEvents())
            reset(event);
    }

    @Deprecated
    public static boolean isTriggered(@EventID int event) {
        if (eventList[event] == null)
            return false;
        return eventList[event].triggered;
    }

    @Deprecated
    public static boolean isTriggered(AutoListener listener) {
        for (int event : listener.getListeningEvents()) {
            if (!isTriggered(event))
                return false;
        }
        return true;
    }

    @Deprecated
    public static <T> T getResult(@EventID int event) {
        return (T) eventList[event].result;
    }

    @Deprecated
    public interface Listener {
        void onEvent(int event);
    }

    @Deprecated
    public interface AutoListener extends Listener {
        @EventID
        int[] getListeningEvents();
    }

    @Deprecated
    private static class Store {
        boolean triggered = false;
        Set<Listener> listeners = new ArraySet<>();
        Object result;
    }
}
