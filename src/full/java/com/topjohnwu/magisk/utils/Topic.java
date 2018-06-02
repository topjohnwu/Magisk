package com.topjohnwu.magisk.utils;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class Topic {

    private static final int NON_INIT = 0;
    private static final int PENDING = 1;
    private static final int PUBLISHED = 2;

    private int state = NON_INIT;
    private List<WeakReference<Subscriber>> subscribers;
    private Object[] results;

    public Topic() {
        subscribers = new SyncArrayList<>();
    }

    public synchronized void subscribe(Subscriber sub) {
        subscribers.add(new WeakReference<>(sub));
    }

    public synchronized void unsubscribe() {
        subscribers = new SyncArrayList<>();
    }

    public synchronized void unsubscribe(Subscriber sub) {
        List<WeakReference<Subscriber>> subs = subscribers;
        subscribers = new ArrayList<>();
        for (WeakReference<Subscriber> subscriber : subs) {
            if (subscriber.get() != null && subscriber.get() != sub)
                subscribers.add(subscriber);
        }
    }

    public void reset() {
        state = NON_INIT;
        results = null;
    }

    public boolean isPublished() {
        return state == PUBLISHED;
    }

    public void publish() {
        publish(true);
    }

    public void publish(boolean record, Object... results) {
        if (record)
            state = PUBLISHED;
        this.results = results;
        // Snapshot
        List<WeakReference<Subscriber>> subs = subscribers;
        for (WeakReference<Subscriber> subscriber : subs) {
            if (subscriber != null && subscriber.get() != null)
                subscriber.get().onTopicPublished(this);
        }
    }

    public Object[] getResults() {
        return results;
    }

    public boolean isPending() {
        return state == PENDING;
    }

    public void setPending() {
        state = PENDING;
    }

    public interface Subscriber {
        default void subscribeTopics() {
            for (Topic topic : getSubscription()) {
                if (topic.isPublished()) {
                    onTopicPublished(topic);
                }
                topic.subscribe(this);
            }
        }
        default void unsubscribeTopics() {
            for (Topic event : getSubscription()) {
                event.unsubscribe(this);
            }
        }
        void onTopicPublished(Topic topic);
        Topic[] getSubscription();
    }

    private static class SyncArrayList<E> extends ArrayList<E> {
        @Override
        public synchronized boolean add(E e) {
            return super.add(e);
        }
    }
}
