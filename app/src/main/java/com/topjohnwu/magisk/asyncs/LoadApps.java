package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.adapters.ApplicationAdapter;
import com.topjohnwu.magisk.utils.Shell;

import java.util.Collections;
import java.util.Iterator;

public class LoadApps extends SerialTask<Void, Void, Void> {

    public LoadApps(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        PackageManager pm = magiskManager.getPackageManager();
        magiskManager.appList = pm.getInstalledApplications(0);
        for (Iterator<ApplicationInfo> i = magiskManager.appList.iterator(); i.hasNext(); ) {
            ApplicationInfo info = i.next();
            if (ApplicationAdapter.BLACKLIST.contains(info.packageName) || !info.enabled)
                i.remove();
        }
        Collections.sort(magiskManager.appList, (a, b) -> a.loadLabel(pm).toString().toLowerCase()
                .compareTo(b.loadLabel(pm).toString().toLowerCase()));
        magiskManager.magiskHideList = Shell.su(MagiskManager.MAGISK_HIDE_PATH + "list");
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.packageLoadDone.trigger();
    }
}
