package com.topjohnwu.magisk.utils;

import android.content.ComponentName;
import android.widget.Toast;

import androidx.core.app.NotificationCompat;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.uicomponents.Notifications;
import com.topjohnwu.signing.JarMap;
import com.topjohnwu.signing.SignAPK;
import com.topjohnwu.superuser.Shell;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.IntBuffer;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;
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
        char next, prev = prefix.charAt(prefix.length() - 1);
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

    private static boolean findAndPatch(byte xml[], String from, String to) {
        if (from.length() != to.length())
            return false;
        CharBuffer buf = ByteBuffer.wrap(xml).order(ByteOrder.LITTLE_ENDIAN).asCharBuffer();
        List<Integer> offList = new ArrayList<>();
        for (int i = 0; i < buf.length() - from.length(); ++i) {
            boolean match = true;
            for (int j = 0; j < from.length(); ++j) {
                if (buf.get(i + j) != from.charAt(j)) {
                    match = false;
                    break;
                }
            }
            if (match) {
                offList.add(i);
                i += from.length();
            }
        }
        if (offList.isEmpty())
            return false;
        for (int off : offList) {
            buf.position(off);
            buf.put(to);
        }
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
        App app = App.self;

        // Generate a new app with random package name
        File repack = new File(app.getFilesDir(), "patched.apk");
        String pkg = genPackageName("com.", BuildConfig.APPLICATION_ID.length());

        if (!patch(app.getPackageCodePath(), repack.getPath(), pkg))
            return false;

        // Install the application
        repack.setReadable(true, false);
        if (!Shell.su("pm install " + repack).exec().isSuccess())
            return false;

        Config.set(Config.Key.SU_MANAGER, pkg);
        Config.export();
        RootUtils.rmAndLaunch(BuildConfig.APPLICATION_ID,
                new ComponentName(pkg, ClassMap.get(SplashActivity.class).getName()));

        return true;
    }

    public static boolean patch(String in, String out, String pkg) {
        try {
            JarMap jar = new JarMap(in);
            JarEntry je = jar.getJarEntry(Const.ANDROID_MANIFEST);
            byte xml[] = jar.getRawData(je);

            if (!findAndPatch(xml, BuildConfig.APPLICATION_ID, pkg) ||
                    !findAndPatch(xml, R.string.app_name, R.string.re_app_name))
                return false;

            // Write in changes
            jar.getOutputStream(je).write(xml);
            SignAPK.sign(jar, new BufferedOutputStream(new FileOutputStream(out)));
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static void hideManager() {
        App.THREAD_POOL.execute(() -> {
            App app = App.self;
            NotificationCompat.Builder progress =
                    Notifications.progress(app.getString(R.string.hide_manager_title));
            Notifications.mgr.notify(Const.ID.HIDE_MANAGER_NOTIFICATION_ID, progress.build());
            if(!patchAndHide())
                Utils.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG);
            Notifications.mgr.cancel(Const.ID.HIDE_MANAGER_NOTIFICATION_ID);
        });
    }
}
