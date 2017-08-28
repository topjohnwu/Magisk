package com.topjohnwu.magisk.utils;

import java.lang.ref.WeakReference;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

public class Topic {

    public boolean hasPublished = false;
    private List<WeakReference<Subscriber>> subscribers;

    public void subscribe(Subscriber sub) {
        if (subscribers == null) {
            subscribers = new LinkedList<>();
        }
        subscribers.add(new WeakReference<>(sub));
    }

    public void unsubscribe() {
        subscribers = null;
    }

    public void unsubscribe(Subscriber sub) {
        for (Iterator<WeakReference<Subscriber>> i = subscribers.iterator(); i.hasNext();) {
            WeakReference<Subscriber> subscriber = i.next();
            if (subscriber.get() == null || subscriber.get() == sub) {
                i.remove();
            }
        }
    }

    public void publish() {
        publish(true);
    }

    public void publish(boolean record) {
        hasPublished = record;
        if (subscribers != null) {
            for (WeakReference<Subscriber> subscriber : subscribers) {
                if (subscriber.get() != null)
                    subscriber.get().onTopicPublished(this);
            }
        }
    }


    public interface Subscriber {
        default void subscribeTopics() {
            for (Topic topic : getSubscription()) {
                if (topic.hasPublished) {
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
        default void onTopicPublished() {
            onTopicPublished(null);
        }
        void onTopicPublished(Topic topic);
        Topic[] getSubscription();
    }
}
