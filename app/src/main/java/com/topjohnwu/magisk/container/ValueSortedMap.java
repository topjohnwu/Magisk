package com.topjohnwu.magisk.container;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ValueSortedMap<K, V extends Comparable<? super V>> extends HashMap<K, V> {

    private List<V> sorted = new ArrayList<>();

    @NonNull
    @Override
    public Collection<V> values() {
        if (sorted.isEmpty()) {
            sorted.addAll(super.values());
            Collections.sort(sorted);
        }
        return sorted;
    }

    @Override
    public V put(K key, V value) {
        sorted.clear();
        return super.put(key, value);
    }

    @Override
    public void putAll(Map<? extends K, ? extends V> m) {
        sorted.clear();
        super.putAll(m);
    }

    @Override
    public V remove(Object key) {
        sorted.clear();
        return super.remove(key);
    }
}
