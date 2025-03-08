package com.topjohnwu.magisk;

import android.content.Context;
import android.content.pm.ApplicationInfo;

public class ProviderInstaller {

    private static final String GMS_PACKAGE_NAME = "com.google.android.gms";

    public static void install(Context context) {
        try {
            // Check if gms is a system app
            ApplicationInfo appInfo = context.getPackageManager().getApplicationInfo(GMS_PACKAGE_NAME, 0);
            if ((appInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
                return;
            }

            // Try installing new SSL provider from Google Play Service
            Context gms = context.createPackageContext(GMS_PACKAGE_NAME,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            gms.getClassLoader()
                    .loadClass("com.google.android.gms.common.security.ProviderInstallerImpl")
                    .getMethod("insertProvider", Context.class)
                    .invoke(null, gms);
        } catch (Exception ignored) {
        }
    }
}
