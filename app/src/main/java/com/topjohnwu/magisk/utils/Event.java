package com.topjohnwu.magisk.utils;

import androidx.annotation.IntDef;
import androidx.collection.ArraySet;

import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Set;

public class Event {

    public static final int MAGISK_HIDE_DONE = 0;
    public static final int MODULE_LOAD_DONE = 1;
    public static final int REPO_LOAD_DONE = 2;
    public static final int UPDATE_CHECK_DONE = 3;
    public static final int LOCALE_FETCH_DONE = 4;

    @IntDef({MAGISK_HIDE_DONE, MODULE_LOAD_DONE, REPO_LOAD_DONE,
            UPDATE_CHECK_DONE, LOCALE_FETCH_DONE})
    @Retention(RetentionPolicy.SOURCE)
    public @interface EventID {}

    // We will not dynamically add topics, so use arrays instead of hash tables
    private static Store[] eventList = new Store[5];

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

    public static void register(AutoListener listener) {
        register(listener, listener.getListeningEvents());
    }

    public static void unregister(Listener listener, @EventID int... events) {
        for (int event : events) {
            if (eventList[event] == null)
                continue;
            eventList[event].listeners.remove(listener);
        }
    }

    public static void unregister(AutoListener listener) {
        unregister(listener, listener.getListeningEvents());
    }

    public static void trigger(@EventID int event) {
        trigger(true, event, null);
    }

    public static void trigger(@EventID int event, Object result) {
        trigger(true, event, result);
    }

    public static void trigger(boolean perm, @EventID int event) {
        trigger(perm, event, null);
    }

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

    public static void reset(@EventID int event) {
        if (eventList[event] == null)
            return;
        eventList[event].triggered = false;
        eventList[event].result = null;
    }

    public static void reset(AutoListener listener) {
        for (int event : listener.getListeningEvents())
            reset(event);
    }

    public static boolean isTriggered(@EventID int event) {
        if (eventList[event] == null)
            return false;
        return eventList[event].triggered;
    }

    public static boolean isTriggered(AutoListener listener) {
        for (int event : listener.getListeningEvents()) {
            if (!isTriggered(event))
                return false;
        }
        return true;
    }

    public static <T> T getResult(@EventID int event) {
        return (T) eventList[event].result;
    }

    private static class Store {
        boolean triggered = false;
        Set<Listener> listeners = new ArraySet<>();
        Object result;
    }

    public interface Listener {
        void onEvent(int event);
    }

    public interface AutoListener extends Listener {
        @EventID
        int[] getListeningEvents();
    }
}
