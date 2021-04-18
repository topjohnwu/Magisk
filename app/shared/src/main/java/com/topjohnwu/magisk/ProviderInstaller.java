package com.topjohnwu.magisk;

import android.content.Context;

public class ProviderInstaller {

    public static boolean install(Context context) {
        try {
            // Try installing new SSL provider from Google Play Service
            Context gms = context.createPackageContext("com.google.android.gms",
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            gms.getClassLoader()
                    .loadClass("com.google.android.gms.common.security.ProviderInstallerImpl")
                    .getMethod("insertProvider", Context.class)
                    .invoke(null, gms);
        } catch (Exception e) {
            return false;
        }
        return true;
    }
}
