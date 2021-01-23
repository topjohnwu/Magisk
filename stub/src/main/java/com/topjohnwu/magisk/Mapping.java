package com.topjohnwu.magisk;

import java.util.HashMap;
import java.util.Map;

public class Mapping {

    private static Map<String, String> map = new HashMap<>();
    public static Map<String, String> inverseMap;

    static {
        map.put("a.Qzw", "a.e");
        map.put("a.u7", "a.c");
        map.put("a.ii", "a.p");
        map.put("a.r", "a.h");
        map.put("xt.R", "a.b");
        map.put("lt5.a", "a.m");
        map.put("d.s", "a.j");
        map.put("w.d", "androidx.work.impl.background.systemjob.SystemJobService");

        inverseMap = new HashMap<>(map.size());
        for (Map.Entry<String, String> e : map.entrySet()) {
            inverseMap.put(e.getValue(), e.getKey());
        }
    }

    public static String get(String name) {
        String n = map.get(name);
        return n != null ? n : name;
    }
}
