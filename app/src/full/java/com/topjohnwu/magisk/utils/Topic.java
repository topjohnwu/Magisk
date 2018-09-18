package com.topjohnwu.magisk.utils;

import com.topjohnwu.magisk.Data;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashSet;
import java.util.Set;

import androidx.annotation.IntDef;

public class Topic {

    public static final int MAGISK_HIDE_DONE = 0;
    public static final int MODULE_LOAD_DONE = 1;
    public static final int REPO_LOAD_DONE = 2;
    public static final int UPDATE_CHECK_DONE = 3;
    public static final int SNET_CHECK_DONE = 4;
    public static final int LOCALE_FETCH_DONE = 5;

    @IntDef({MAGISK_HIDE_DONE, MODULE_LOAD_DONE, REPO_LOAD_DONE,
            UPDATE_CHECK_DONE, SNET_CHECK_DONE, LOCALE_FETCH_DONE})
    @Retention(RetentionPolicy.SOURCE)
    public @interface TopicID {}

    // We will not dynamically add topics, so use arrays instead of hash tables
    private static Store[] topicList = new Store[6];

    public static void subscribe(Subscriber sub, @TopicID int... topics) {
        for (int topic : topics) {
            if (topicList[topic] == null)
                topicList[topic] = new Store();
            topicList[topic].subscribers.add(sub);
            if (topicList[topic].published) {
                sub.onPublish(topic, topicList[topic].results);
            }
        }
    }

    public static void subscribe(AutoSubscriber sub) {
        if (sub instanceof Subscriber)
            subscribe((Subscriber) sub, sub.getSubscribedTopics());
    }

    public static void unsubscribe(Subscriber sub, @TopicID int... topics) {
        for (int topic : topics) {
            if (topicList[topic] == null)
                continue;
            topicList[topic].subscribers.remove(sub);
        }
    }

    public static void unsubscribe(AutoSubscriber sub) {
        if (sub instanceof Subscriber)
            unsubscribe((Subscriber) sub, sub.getSubscribedTopics());
    }

    public static void publish(@TopicID int topic, Object... results) {
        publish(true, topic, results);
    }

    public static void publish(boolean persist, @TopicID int topic, Object... results) {
        if (topicList[topic] == null)
            topicList[topic] = new Store();
        if (persist) {
            topicList[topic].results = results;
            topicList[topic].published = true;
        }
        for (Subscriber sub : topicList[topic].subscribers) {
            Data.mainHandler.post(() -> sub.onPublish(topic, results));
        }
    }

    public static void reset(@TopicID int... topics) {
        for (int topic : topics) {
            if (topicList[topic] == null)
                continue;
            topicList[topic].published = false;
            topicList[topic].results = null;
        }
    }

    public static boolean isPublished(@TopicID int... topics) {
        for (int topic : topics) {
            if (topicList[topic] == null)
                return false;
            if (!topicList[topic].published)
                return false;
        }
        return true;
    }

    private static class Store {
        boolean published = false;
        Set<Subscriber> subscribers = new HashSet<>();
        Object[] results;
    }

    public interface Subscriber {
        void onPublish(int topic, Object[] result);
    }

    public interface AutoSubscriber {
        @TopicID
        int[] getSubscribedTopics();
    }
}
