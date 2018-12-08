package com.topjohnwu.magisk.asyncs;

import android.os.AsyncTask;
import android.widget.Toast;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.Notifications;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.utils.JarMap;
import com.topjohnwu.utils.SignAPK;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.IntBuffer;
import java.security.SecureRandom;
import java.util.jar.JarEntry;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

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

    private static boolean findAndPatch(byte xml[], int a, int b) {
        IntBuffer buf = ByteBuffer.wrap(xml).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer();
        int len = xml.length / 4;
        for (int i = 0; i < len; ++i) {
            if (buf.get(i) == a) {
                buf.put(i, b);
                return true;
            }
        }
        return false;
    }

    private static boolean patchAndHide() {
        MagiskManager mm = Data.MM();

        // Generate a new app with random package name
        File repack = new File(mm.getFilesDir(), "patched.apk");
        String pkg = genPackageName("com.", BuildConfig.APPLICATION_ID.length());

        try {
            JarMap apk = new JarMap(mm.getPackageCodePath());
            if (!patch(apk, pkg))
                return false;
            SignAPK.sign(apk, new BufferedOutputStream(new FileOutputStream(repack)));
        } catch (Exception e) {
            return false;
        }

        // Install the application
        repack.setReadable(true, false);
        if (!ShellUtils.fastCmdResult("pm install " + repack))
            return false;;

        mm.mDB.setStrings(Const.Key.SU_MANAGER, pkg);
        Data.exportPrefs();
        RootUtils.rmAndLaunch(BuildConfig.APPLICATION_ID, pkg);

        return true;
    }

    public static boolean patch(JarMap apk, String pkg) {
        try {
            JarEntry je = apk.getJarEntry(Const.ANDROID_MANIFEST);
            byte xml[] = apk.getRawData(je);

            if (!findAndPatch(xml, BuildConfig.APPLICATION_ID, pkg) ||
                    !findAndPatch(xml, BuildConfig.APPLICATION_ID + ".provider", pkg + ".provider") ||
                    !findAndPatch(xml, R.string.app_name, R.string.re_app_name))
                return false;

            // Write in changes
            apk.getOutputStream(je).write(xml);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static void hideManager() {
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            MagiskManager mm = Data.MM();
            NotificationCompat.Builder progress =
                    Notifications.progress(mm.getString(R.string.hide_manager_title));
            NotificationManagerCompat mgr = NotificationManagerCompat.from(mm);
            mgr.notify(Const.ID.HIDE_MANAGER_NOTIFICATION_ID, progress.build());
            boolean b = patchAndHide();
            mgr.cancel(Const.ID.HIDE_MANAGER_NOTIFICATION_ID);
            if (!b) Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG);
        });
    }
}
