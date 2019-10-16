package com.topjohnwu.magisk;

import java.util.HashMap;
import java.util.Map;

class ComponentMap {
    private static Map<String, String> map = new HashMap<>();

    // This mapping will be sent into the guest app
    static Map<String, String> inverseMap;

    static {
        map.put(a.z.class.getName(), "a.c");
        map.put("a.x", "a.f");
        map.put("a.o", "a.b");
        map.put("a.g", "a.m");
        map.put(a.w.class.getName(), "a.h");
        map.put("a.v", "a.j");
        map.put("a.j", "androidx.work.impl.background.systemjob.SystemJobService");

        inverseMap = new HashMap<>(map.size());
        for (Map.Entry<String, String> e : map.entrySet()) {
            inverseMap.put(e.getValue(), e.getKey());
        }
    }

    static String get(String name) {
        String n = map.get(name);
        return n != null ? n : name;
    }

}
