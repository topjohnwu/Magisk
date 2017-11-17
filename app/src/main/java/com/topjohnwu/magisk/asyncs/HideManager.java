package com.topjohnwu.magisk.asyncs;

import android.widget.Toast;

import com.topjohnwu.crypto.JarMap;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;
import java.io.FileInputStream;
import java.security.SecureRandom;
import java.util.List;
import java.util.Locale;
import java.util.jar.JarEntry;

public class HideManager extends ParallelTask<Void, Void, Boolean> {

    private String genPackageName(String prefix, int length) {
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

    private int findOffset(byte buf[], byte pattern[]) {
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
    private boolean fallbackPatch(byte xml[], String from, String to) {

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

    private boolean findAndPatch(byte xml[], String from, String to) {
        byte target[] = (from + '\0').getBytes();
        int offset = findOffset(xml, target);
        if (offset < 0)
            return fallbackPatch(xml, from, to);
        System.arraycopy(to.getBytes(), 0, xml, offset, to.length());
        return true;
    }

    @Override
    protected void onPreExecute() {
        MagiskManager.toast(R.string.hide_manager_toast, Toast.LENGTH_SHORT);
        MagiskManager.toast(R.string.hide_manager_toast2, Toast.LENGTH_LONG);
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        MagiskManager mm = MagiskManager.get();

        // Generate a new unhide app with random package name
        File repack = new File(Const.EXTERNAL_PATH, "repack.apk");
        repack.getParentFile().mkdirs();
        String pkg = genPackageName("com.", Const.ORIG_PKG_NAME.length());

        try {
            // Read whole APK into memory
            JarMap apk = new JarMap(new FileInputStream(mm.getPackageCodePath()));
            JarEntry je = new JarEntry(Const.ANDROID_MANIFEST);
            byte xml[] = apk.getRawData(je);

            if (!findAndPatch(xml, Const.ORIG_PKG_NAME, pkg))
                return false;
            if (!findAndPatch(xml, Const.ORIG_PKG_NAME + ".provider", pkg + ".provider"))
                return false;

            // Write in changes
            apk.getOutputStream(je).write(xml);

            // Sign the APK
            ZipUtils.signZip(apk, repack, false);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        // Install the application

        List<String> ret = Shell.su(String.format(Locale.US,
                "pm install --user %d %s >/dev/null && echo true || echo false",
                mm.userId, repack));
        repack.delete();
        if (!Utils.isValidShellResponse(ret) || !Boolean.parseBoolean(ret.get(0)))
            return false;

        mm.suDB.setStrings(Const.Key.SU_REQUESTER, pkg);
        Shell.su_raw(String.format(Locale.US, "pm uninstall --user %d %s", mm.userId, mm.getPackageName()));

        return true;
    }

    @Override
    protected void onPostExecute(Boolean b) {
        if (!b) {
            MagiskManager.toast(R.string.hide_manager_fail_toast, Toast.LENGTH_LONG);
        }
        super.onPostExecute(b);
    }
}
