package com.topjohnwu.magisk;

import com.topjohnwu.core.App;
import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.receivers.GeneralReceiver;
import com.topjohnwu.magisk.receivers.ShortcutReceiver;
import com.topjohnwu.magisk.services.DownloadModuleService;
import com.topjohnwu.magisk.services.UpdateCheckService;

import java.util.HashMap;
import java.util.Map;

public class ClassMap {
    private static Map<Class, Class> classMap = new HashMap<>();

    static {
        classMap.put(App.class, a.q.class);
        classMap.put(MainActivity.class, a.b.class);
        classMap.put(SplashActivity.class, a.c.class);
        classMap.put(AboutActivity.class, a.d.class);
        classMap.put(DonationActivity.class, a.e.class);
        classMap.put(FlashActivity.class, a.f.class);
        classMap.put(UpdateCheckService.class, a.g.class);
        classMap.put(GeneralReceiver.class, a.h.class);
        classMap.put(ShortcutReceiver.class, a.i.class);
        classMap.put(DownloadModuleService.class, a.j.class);
        classMap.put(AboutCardRow.class, a.l.class);
        classMap.put(SuRequestActivity.class, a.m.class);
    }
    
    public static <T> Class<T> get(Class c) {
        return classMap.get(c);
    }
}
