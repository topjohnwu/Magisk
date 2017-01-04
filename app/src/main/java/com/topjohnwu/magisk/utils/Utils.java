package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.DownloadManager;
import android.content.Context;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.util.Base64;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.List;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

public class Utils {

    public static boolean isDownloading = false;

    private static final String cryptoPass = "MagiskRox666";
    private static final String secret = "GTYybRBTYf5his9kQ16ZNO7qgkBJ/5MyVe4CGceAOIoXgSnnk8FTd4F1dE9p5Eus";

    public static boolean itemExist(String path) {
        return itemExist(true, path);
    }

    public static boolean itemExist(boolean root, String path) {
        String command = "if [ -e " + path + " ]; then echo true; else echo false; fi";
        if (Shell.rootAccess() && root) {
            return Boolean.parseBoolean(Shell.su(command).get(0));
        } else {
            return new File(path).exists();
        }
    }

    public static boolean commandExists(String s) {
        List<String> ret;
        String command = "if [ -z $(which " + s + ") ]; then echo false; else echo true; fi";
        ret = Shell.sh(command);
        return Boolean.parseBoolean(ret.get(0));
    }

    public static boolean createFile(String path) {
        String folder = path.substring(0, path.lastIndexOf('/'));
        String command = "mkdir -p " + folder + " 2>/dev/null; touch " + path + " 2>/dev/null; if [ -f \"" + path + "\" ]; then echo true; else echo false; fi";
        return Shell.rootAccess() && Boolean.parseBoolean(Shell.su(command).get(0));
    }

    public static boolean removeItem(String path) {
        String command = "rm -rf " + path + " 2>/dev/null; if [ -e " + path + " ]; then echo false; else echo true; fi";
        return Shell.rootAccess() && Boolean.parseBoolean(Shell.su(command).get(0));
    }

    public static List<String> getModList(String path) {
        List<String> ret;
        String command = "find " + path + " -type d -maxdepth 1 ! -name \"*.core\" ! -name \"*lost+found\" ! -name \"*magisk\"";
        if (Shell.rootAccess()) {
            ret = Shell.su(command);
        } else {
            ret = Shell.sh(command);
        }
        return ret;
    }

    public static List<String> readFile(String path) {
        List<String> ret;
        String command = "cat " + path;
        if (Shell.rootAccess()) {
            ret = Shell.su(command);
        } else {
            ret = Shell.sh(command);
        }
        return ret;
    }

    public static void dlAndReceive(Context context, DownloadReceiver receiver, String link, String filename) {
        if (isDownloading) {
            return;
        }

        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
            return;
        }

        File file = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + filename);

        if ((!file.getParentFile().exists() && !file.getParentFile().mkdirs()) || (file.exists() && !file.delete())) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
            return;
        }

        Toast.makeText(context, context.getString(R.string.downloading_toast, filename), Toast.LENGTH_LONG).show();
        isDownloading = true;

        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        DownloadManager.Request request = new DownloadManager.Request(Uri.parse(link));
        request.setDestinationUri(Uri.fromFile(file));

        receiver.setDownloadID(downloadManager.enqueue(request));
        receiver.setFilename(filename);
        context.registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
    }

    public static String getToken() {

        try {
            DESKeySpec keySpec = new DESKeySpec(cryptoPass.getBytes("UTF8"));
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
            SecretKey key = keyFactory.generateSecret(keySpec);

            byte[] encrypedPwdBytes = Base64.decode(secret, Base64.DEFAULT);
            // cipher is not thread safe
            Cipher cipher = Cipher.getInstance("DES");
            cipher.init(Cipher.DECRYPT_MODE, key);
            byte[] decrypedValueBytes = (cipher.doFinal(encrypedPwdBytes));

            return new String(decrypedValueBytes);

        } catch (InvalidKeyException | UnsupportedEncodingException | NoSuchAlgorithmException
                | BadPaddingException | NoSuchPaddingException | IllegalBlockSizeException
                | InvalidKeySpecException e) {
            e.printStackTrace();
        }
        return secret;
    }

    public static String getLegalFilename(CharSequence filename) {
        return filename.toString().replace(" ", "_").replace("'", "").replace("\"", "")
                .replace("$", "").replace("`", "").replace("(", "").replace(")", "")
                .replace("#", "").replace("@", "").replace("*", "");
    }

    public static String detectBootImage() {
        String[] commands = {
                "for PARTITION in kern-a KERN-A android_boot ANDROID_BOOT kernel KERNEL boot BOOT lnx LNX; do",
                "BOOTIMAGE=`readlink /dev/block/by-name/$PARTITION || readlink /dev/block/platform/*/by-name/$PARTITION || readlink /dev/block/platform/*/*/by-name/$PARTITION`",
                "if [ ! -z \"$BOOTIMAGE\" ]; then break; fi",
                "done",
                "echo \"${BOOTIMAGE##*/}\""
        };
        List<String> ret = Shell.su(commands);
        if (!ret.isEmpty()) {
            return ret.get(0);
        }
        return null;
    }

    public static boolean isDarkTheme(String theme, Context resources) {
        return theme != null && theme.equalsIgnoreCase(resources.getString(R.string.theme_dark_value));
    }

    public static class ByteArrayInOutStream extends ByteArrayOutputStream {
        public ByteArrayInputStream getInputStream() {
            ByteArrayInputStream in = new ByteArrayInputStream(buf, 0, count);
            count = 0;
            buf = new byte[32];
            return in;
        }
        public void setBuffer(byte[] buffer) {
            buf = buffer;
            count = buffer.length;
        }
    }

}