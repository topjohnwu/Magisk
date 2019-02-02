package com.topjohnwu.magisk.container;

import android.content.ContentValues;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;

import com.topjohnwu.magisk.utils.Utils;

import androidx.annotation.NonNull;


public class Policy implements Comparable<Policy>{
    public static final int INTERACTIVE = 0;
    public static final int DENY = 1;
    public static final int ALLOW = 2;

    public int uid, policy = INTERACTIVE;
    public long until;
    public boolean logging = true, notification = true;
    public String packageName, appName;
    public ApplicationInfo info;

    public Policy(int uid, PackageManager pm) throws PackageManager.NameNotFoundException {
        String[] pkgs = pm.getPackagesForUid(uid);
        if (pkgs == null || pkgs.length == 0)
            throw new PackageManager.NameNotFoundException();
        this.uid = uid;
        packageName = pkgs[0];
        info = pm.getApplicationInfo(packageName, 0);
        appName = Utils.getAppLabel(info, pm);
    }

    public Policy(ContentValues values, PackageManager pm) throws PackageManager.NameNotFoundException {
        uid = values.getAsInteger("uid");
        packageName = values.getAsString("package_name");
        policy = values.getAsInteger("policy");
        until = values.getAsInteger("until");
        logging = values.getAsInteger("logging") != 0;
        notification = values.getAsInteger("notification") != 0;
        info = pm.getApplicationInfo(packageName, 0);
        if (info.uid != uid)
            throw new PackageManager.NameNotFoundException();
        appName = info.loadLabel(pm).toString();
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("uid", uid);
        values.put("package_name", packageName);
        values.put("policy", policy);
        values.put("until", until);
        values.put("logging", logging ? 1 : 0);
        values.put("notification", notification ? 1 : 0);
        return values;
    }

    @Override
    public int compareTo(@NonNull Policy policy) {
        return appName.toLowerCase().compareTo(policy.appName.toLowerCase());
    }
}
