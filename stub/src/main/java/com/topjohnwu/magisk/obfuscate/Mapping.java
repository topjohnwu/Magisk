package com.topjohnwu.magisk.obfuscate;

import java.util.HashMap;
import java.util.Map;

import static com.topjohnwu.magisk.DynAPK.Data;

public class Mapping {
    private static final int STUB_VERSION = 7;

    private static Map<String, String> map = new HashMap<>();
    private static Map<String, String> inverseMap;

    static {
        map.put("a.x", "androidx.work.impl.background.systemjob.SystemJobService");
        inverseMap = new HashMap<>(map.size());
        for (Map.Entry<String, String> e : map.entrySet()) {
            inverseMap.put(e.getValue(), e.getKey());
        }
    }

    public static String get(String name) {
        String n = map.get(name);
        return n != null ? n : name;
    }

    public static String inverse(String name) {
        String n = inverseMap.get(name);
        return n != null ? n : name;
    }

    public static Data data() {
        Data data = new Data();
        data.version = STUB_VERSION;
        data.classToComponent = inverseMap;
        return data;
    }
}
