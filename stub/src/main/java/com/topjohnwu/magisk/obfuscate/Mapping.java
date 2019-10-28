package com.topjohnwu.magisk.obfuscate;

import java.util.HashMap;
import java.util.Map;

import static com.topjohnwu.magisk.DynAPK.*;
import static com.topjohnwu.magisk.R.drawable.*;

public class Mapping {
    private static Map<String, String> map = new HashMap<>();

    // This mapping will be sent into the guest app
    public static Data data = new Data();

    static {
        map.put(a.z.class.getName(), "a.c");
        map.put("a.x", "a.f");
        map.put("a.o", "a.b");
        map.put("a.g", "a.m");
        map.put(a.w.class.getName(), "a.h");
        map.put("a.v", "a.j");
        map.put("a.j", "androidx.work.impl.background.systemjob.SystemJobService");

        data.componentMap = new HashMap<>(map.size());
        for (Map.Entry<String, String> e : map.entrySet()) {
            data.componentMap.put(e.getValue(), e.getKey());
        }
        int[] res = new int[5];
        res[NOTIFICATION] = ic_magisk_outline;
        res[SUPERUSER] = sc_superuser;
        res[MAGISKHIDE] = sc_magiskhide;
        res[DOWNLOAD] = sc_cloud_download;
        res[MODULES] = sc_extension;
        data.resourceMap = res;
    }

    public static String get(String name) {
        String n = map.get(name);
        return n != null ? n : name;
    }

}
