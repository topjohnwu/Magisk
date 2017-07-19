package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.adapters.ApplicationAdapter;

import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class LoadApps extends ParallelTask<Void, Void, Void> {

    public LoadApps(Context context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return null;
        PackageManager pm = magiskManager.getPackageManager();
        List<ApplicationInfo> list = pm.getInstalledApplications(0);
        for (Iterator<ApplicationInfo> i = list.iterator(); i.hasNext(); ) {
            ApplicationInfo info = i.next();
            if (ApplicationAdapter.BLACKLIST.contains(info.packageName) || !info.enabled) {
                i.remove();
            }
        }
        Collections.sort(list, (a, b) -> a.loadLabel(pm).toString().toLowerCase()
                .compareTo(b.loadLabel(pm).toString().toLowerCase()));
        magiskManager.appList = Collections.unmodifiableList(list);
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return;
        new MagiskHide(magiskManager).list();
    }
}
