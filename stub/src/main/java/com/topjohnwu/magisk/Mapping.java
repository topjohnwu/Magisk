package com.topjohnwu.magisk;

import com.topjohnwu.magisk.dummy.DummyReceiver;

import java.util.HashMap;
import java.util.Map;

/**
 * These are just some random class names hardcoded as an example.
 * For the actual release builds, these mappings will be auto generated.
 */
public class Mapping {

    private static final Map<String, String> map = new HashMap<>();
    public static final Map<String, Class<?>> internalMap = new HashMap<>();
    public static final Map<String, String> inverseMap;

    static {
        map.put("a.Q", "com.topjohnwu.magisk.core.App");
        map.put("f.u7", "com.topjohnwu.magisk.core.SplashActivity");
        map.put("fxQ.lk", "com.topjohnwu.magisk.core.Provider");
        map.put("yy.E", "com.topjohnwu.magisk.core.Receiver");
        map.put("xt.R", "com.topjohnwu.magisk.ui.MainActivity");
        map.put("lt5.a", "com.topjohnwu.magisk.ui.surequest.SuRequestActivity");
        map.put("d.s", "com.topjohnwu.magisk.core.download.DownloadService");
        map.put("w.d", "androidx.work.impl.background.systemjob.SystemJobService");

        internalMap.put("a.Q", DelegateApplication.class);
        internalMap.put("f.u7", DownloadActivity.class);
        internalMap.put("fxQ.lk", FileProvider.class);
        internalMap.put("yy.E", DummyReceiver.class);

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
