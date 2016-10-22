package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.DownloadManager;
import android.content.Context;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.v4.app.ActivityCompat;
import android.util.Base64;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.ArrayList;
import java.util.List;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

public class Utils {

    private static final String TAG = "Magisk";

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

    public static String getAppUID(String packageName) {
        List<String> retString = Shell.su("ls -nld /data/data/" + packageName);
        String splitMe = retString.get(0);
        String[] splitString = retString.get(0).split(" ");
        return splitString[5];
    }

    public static int WhichHide(Context context) {
        Boolean mh = PreferenceManager.getDefaultSharedPreferences(context).getBoolean("magiskhide", false);
        Boolean sh = Utils.itemExist("/su/suhide/add");
        if (mh && !sh) {
            return 1;
        }
        if (sh && !mh) {
            return 2;
        }
        if (sh && mh) {
            return 3;
        }
        return 0;

    }

    public static boolean commandExists(String s) {
        List<String> ret;
        String command = "if [ -z $(which " + s + ") ]; then echo false; else echo true; fi";
        ret = Shell.sh(command);
        return Boolean.parseBoolean(ret.get(0));
    }

    public static boolean rootEnabled() {
        return commandExists("su");
    }

    public static boolean autoToggleEnabled(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        Logger.dev("Utils: AutoRootEnableCheck is " + preferences.getBoolean("autoRootEnable", false));
        return PreferenceManager.getDefaultSharedPreferences(context).getBoolean("autoRootEnable", false);

    }

    public static boolean createFile(String path) {
        String command = "touch " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo true; else echo false; fi";
        return Shell.rootAccess() && Boolean.parseBoolean(Shell.su(command).get(0));
    }

    public static boolean removeItem(String path) {
        String command = "rm -rf " + path + " 2>/dev/null; if [ -e " + path + " ]; then echo false; else echo true; fi";
        return Shell.rootAccess() && Boolean.parseBoolean(Shell.su(command).get(0));
    }

    static List<String> getModList(String path) {
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

    public static void downloadAndReceive(Context context, DownloadReceiver receiver, String link, String filename) {
        File file = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + filename);
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
            return;
        }

        if ((!file.getParentFile().exists() && !file.getParentFile().mkdirs()) || (file.exists() && !file.delete())) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
        }

        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        DownloadManager.Request request = new DownloadManager.Request(Uri.parse(link));
        request.setDestinationUri(Uri.fromFile(file));

        receiver.setDownloadID(downloadManager.enqueue(request));
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

}