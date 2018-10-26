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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.security.SecureRandom;
import java.util.jar.JarEntry;

public class PatchAPK {

    public static final String LOWERALPHA = "abcdefghijklmnopqrstuvwxyz";
    public static final String UPPERALPHA = LOWERALPHA.toUpperCase();
    public static final String ALPHA = LOWERALPHA + UPPERALPHA;
    public static final String DIGITS = "0123456789";
    public static final String ALPHANUM = ALPHA + DIGITS;
    public static final String ALPHANUMDOTS = ALPHANUM + "............";

    private static String genPackageName(String prefix, int length) {
        StringBuilder builder = new StringBuilder(length);
        builder.append(prefix);
        length -= prefix.length();
        SecureRandom random = new SecureRandom();
        char next, prev = '.';
        for (int i = 0; i < length; ++i) {
            if (prev == '.' || i == length - 1) {
                next = ALPHA.charAt(random.nextInt(ALPHA.length()));
            } else {
                next = ALPHANUMDOTS.charAt(random.nextInt(ALPHANUMDOTS.length()));
            }
            builder.append(next);
            prev = next;
        }
        return builder.toString();
    }

    private static boolean findAndPatch(byte xml[], String a, String b) {
        if (a.length() != b.length())
            return false;
        char[] from = a.toCharArray(), to = b.toCharArray();
        CharBuffer buf = ByteBuffer.wrap(xml).order(ByteOrder.LITTLE_ENDIAN).asCharBuffer();
        int offset = -1;
        for (int i = 0; i < buf.length() - from.length; ++i) {
            boolean match = true;
            for (int j = 0; j < from.length; ++j) {
                if (buf.get(i + j) != from[j]) {
                    match = false;
                    break;
                }
            }
            // Make sure it is null terminated
            if (match && buf.get(i + from.length) == '\0') {
                offset = i;
                break;
            }
        }
        if (offset < 0)
            return false;
        buf.position(offset);
        buf.put(to);
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

            if (!findAndPatch(xml, from, to) ||
                    !findAndPatch(xml, from + ".provider", to + ".provider"))
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
