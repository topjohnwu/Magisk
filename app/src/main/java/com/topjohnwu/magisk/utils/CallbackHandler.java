package com.topjohnwu.magisk.utils;

import java.util.HashMap;
import java.util.HashSet;

public class CallbackHandler {

    private static HashMap<Event, HashSet<EventListener>> listeners = new HashMap<>();

    public static void register(Event event, EventListener listener) {
        HashSet<EventListener> list = listeners.get(event);
        if (list == null) {
            list = new HashSet<>();
            listeners.put(event, list);
        }
        list.add(listener);
    }

    public static void unRegister(Event event, EventListener listener) {
        HashSet<EventListener> list = listeners.get(event);
        if (list != null) {
            list.remove(listener);
        }
    }

    private static void triggerCallback(Event event) {
        HashSet<EventListener> list = listeners.get(event);
        if (list != null) {
            for (EventListener listener : list) {
                listener.onTrigger(event);
            }
        }
    }

    public static class Event {

        public boolean isTriggered = false;
        private Object result;

        public void trigger() {
            trigger(null);
        }

        public void trigger(Object result) {
            this.result = result;
            isTriggered = true;
            triggerCallback(this);
        }

        public Object getResult() {
            return result;
        }
    }

    public interface EventListener {
        void onTrigger(Event event);
    }
}
