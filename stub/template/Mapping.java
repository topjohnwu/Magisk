package com.topjohnwu.magisk;

import java.util.HashMap;
import java.util.Map;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public class Mapping {

    private static final Map<String, String> map = new HashMap<>();
    public static final Map<String, Class<?>> internalMap = new HashMap<>();
    public static final Map<String, String> inverseMap = new HashMap<>();

    static {
%s
        for (Map.Entry<String, String> e : map.entrySet()) {
            inverseMap.put(e.getValue(), e.getKey());
        }
    }

    public static String get(String name) {
        String n = map.get(name);
        return n != null ? n : name;
    }
}
