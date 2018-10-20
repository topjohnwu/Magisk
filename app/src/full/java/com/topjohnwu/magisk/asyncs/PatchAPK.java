package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileOutputStream;
import com.topjohnwu.utils.JarMap;
import com.topjohnwu.utils.SignAPK;

import java.security.SecureRandom;
import java.util.jar.JarEntry;

public class PatchAPK {

    private static String genPackageName(String prefix, int length) {
        StringBuilder builder = new StringBuilder(length);
        builder.append(prefix);
        length -= prefix.length();
        SecureRandom random = new SecureRandom();
        String base = "abcdefghijklmnopqrstuvwxyz";
        String alpha = base + base.toUpperCase();
        String full = alpha + "0123456789..........";
        char next, prev = '\0';
        for (int i = 0; i < length; ++i) {
            if (prev == '.' || i == length - 1 || i == 0) {
                next = alpha.charAt(random.nextInt(alpha.length()));
            } else {
                next = full.charAt(random.nextInt(full.length()));
            }
            builder.append(next);
            prev = next;
        }
        return builder.toString();
    }

    private static int findOffset(byte buf[], byte pattern[]) {
        int offset = -1;
        for (int i = 0; i < buf.length - pattern.length; ++i) {
            boolean match = true;
            for (int j = 0; j < pattern.length; ++j) {
                if (buf[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                offset = i;
                break;
            }
        }
        return offset;
    }

    /* It seems that AAPT sometimes generate another type of string format */
    private static boolean fallbackPatch(byte xml[], String from, String to) {

        byte[] target = new byte[from.length() * 2 + 2];
        for (int i = 0; i < from.length(); ++i) {
            target[i * 2] = (byte) from.charAt(i);
        }
        int offset = findOffset(xml, target);
        if (offset < 0)
            return false;
        byte[] dest = new byte[target.length - 2];
        for (int i = 0; i < to.length(); ++i) {
            dest[i * 2] = (byte) to.charAt(i);
        }
        System.arraycopy(dest, 0, xml, offset, dest.length);
        return true;
    }

    private static boolean findAndPatch(byte xml[], String from, String to) {
        byte target[] = (from + '\0').getBytes();
        int offset = findOffset(xml, target);
        if (offset < 0)
            return fallbackPatch(xml, from, to);
        System.arraycopy(to.getBytes(), 0, xml, offset, to.length());
        return true;
    }

    private static boolean patchAndHide() {
        MagiskManager mm = Data.MM();

        // Generate a new app with random package name
        SuFile repack = new SuFile("/data/local/tmp/repack.apk");
        String pkg = genPackageName("com.", Const.ORIG_PKG_NAME.length());

        try {
            JarMap apk = new JarMap(mm.getPackageCodePath());
            if (!patchPackageID(apk, Const.ORIG_PKG_NAME, pkg))
                return false;
            SignAPK.sign(apk, new SuFileOutputStream(repack));
        } catch (Exception e) {
            return false;
        }

        // Install the application
        if (!ShellUtils.fastCmdResult("pm install " + repack))
            return false;

        repack.delete();

        mm.mDB.setStrings(Const.Key.SU_MANAGER, pkg);
        mm.mDB.flush();
        Data.exportPrefs();
        RootUtils.uninstallPkg(Const.ORIG_PKG_NAME);

        return true;
    }

    public static boolean patchPackageID(JarMap apk, String from, String to) {
        try {
            JarEntry je = apk.getJarEntry(Const.ANDROID_MANIFEST);
            byte xml[] = apk.getRawData(je);

            if (!findAndPatch(xml, from, to))
                return false;
            if (!findAndPatch(xml, from + ".provider", to + ".provider"))
                return false;

            // Write in changes
            apk.getOutputStream(je).write(xml);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static void hideManager(Activity activity) {
        ProgressDialog dialog = ProgressDialog.show(activity,
                activity.getString(R.string.hide_manager_toast),
                activity.getString(R.string.hide_manager_toast2));
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            boolean b = patchAndHide();
            Data.mainHandler.post(() -> {
                dialog.cancel();
                if (!b) {
                    Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG);
                }
            });
        });
    }
}
