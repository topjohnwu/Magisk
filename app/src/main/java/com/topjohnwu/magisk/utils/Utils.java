package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.ActivityManager;
import android.app.DownloadManager;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.kcoppock.broadcasttilesupport.BroadcastTileIntentBuilder;
import com.topjohnwu.magisk.MagiskFragment;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.PrivateBroadcastReceiver;
import com.topjohnwu.magisk.services.MonitorService;
import com.topjohnwu.magisk.services.TileServiceCompat;
import com.topjohnwu.magisk.services.TileServiceNewApi;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.LinkedList;
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

    public static void init(Context context) {
        List<String> ret = Shell.sh("getprop magisk.version");
        if (ret.get(0).isEmpty()) {
            MagiskFragment.magiskVersion = -1;
        } else {
            MagiskFragment.magiskVersion = Integer.parseInt(ret.get(0));
        }
        String toolPath = context.getApplicationInfo().dataDir + "/tools";
        Shell.su("PATH=" + toolPath + ":$PATH");
    }

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

    public static boolean removeFile(String path) {
        String command = "rm -f " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo false; else echo true; fi";
        return Shell.rootAccess() && Boolean.parseBoolean(Shell.su(command).get(0));
    }

    public static void toggleRoot(Boolean b, Context context) {
        if (MagiskFragment.magiskVersion != -1) {
            if (b) {
                Shell.su("ln -s $(getprop magisk.supath) /magisk/.core/bin", "setprop magisk.root 1");
            } else {
                Shell.su("rm -rf /magisk/.core/bin", "setprop magisk.root 0");
            }
            if (PreferenceManager.getDefaultSharedPreferences(context).getBoolean("enable_quicktile", false)) {
                setupQuickSettingsTile(context);
            }
            PreferenceManager.getDefaultSharedPreferences(context).edit().putBoolean("root", b).apply();
        }
    }

    public static void toggleAutoRoot(Boolean b, Context context) {
        Logger.dev("Utils: toggleAutocalled for " + b );
        if (MagiskFragment.magiskVersion != -1) {
            if (!Utils.hasServicePermission(context)) {
                Intent intent = new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS);
                Toast.makeText(context, "Please enable Magisk in accessibility for auto-toggle work.", Toast.LENGTH_LONG).show();
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
            } else {
                Logger.dev("Utils: toggleAuto checks passed, setting" + b );
                PreferenceManager.getDefaultSharedPreferences(context).edit().putBoolean("autoRootEnable", b).apply();
                Intent myServiceIntent = new Intent(context, MonitorService.class);
                myServiceIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                if (b) {
                    context.startService(myServiceIntent);
                } else {
                    context.stopService(myServiceIntent);
                }
            }
        }
        if (PreferenceManager.getDefaultSharedPreferences(context).getBoolean("enable_quicktile", false)) {
            setupQuickSettingsTile(context);
        }
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

    public static void setupQuickSettingsTile(Context mContext) {
        Logger.dev("Utils: SetupQuickSettings called");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            Logger.dev("Utils: Starting N quick settings service");
            Intent serviceIntent = new Intent(mContext, TileServiceNewApi.class);
            mContext.startService(serviceIntent);

        }
        if (Build.VERSION.SDK_INT == Build.VERSION_CODES.M) {
            Logger.dev("Utils: Marshmallow build detected");
            String mLabelString;
            int mRootIcon = R.drawable.root_white;
            int mAutoRootIcon = R.drawable.ic_autoroot_white;
            int mRootDisabled = R.drawable.root_grey;
            int mRootsState = CheckRootsState(mContext);
            Logger.dev("Utils: Root State returned as " + mRootsState);
            final Intent enableBroadcast = new Intent(PrivateBroadcastReceiver.ACTION_ENABLEROOT);
            final Intent disableBroadcast = new Intent(PrivateBroadcastReceiver.ACTION_DISABLEROOT);
            final Intent autoBroadcast = new Intent(PrivateBroadcastReceiver.ACTION_AUTOROOT);
            Intent intent;

            int mIcon;
            switch (mRootsState) {
                case 2:
                    mLabelString = mContext.getString(R.string.auto_toggle);
                    mIcon = mAutoRootIcon;
                    intent = autoBroadcast;
                    break;
                case 1:
                    mLabelString = "Root enabled";
                    mIcon = mRootIcon;
                    intent = disableBroadcast;
                    break;
                case 0:
                    mLabelString = "Root disabled";
                    mIcon = mRootDisabled;
                    intent = enableBroadcast;
                    break;
                default:
                    mLabelString = "Root disabled";
                    mIcon = mRootDisabled;
                    intent = disableBroadcast;
                    break;
            }

            Intent tileConfigurationIntent = new BroadcastTileIntentBuilder(mContext, "Magisk")
                    .setLabel(mLabelString)
                    .setIconResource(mIcon)
                    .setOnClickBroadcast(intent)
                    .setOnLongClickBroadcast(autoBroadcast)
                    .setVisible(true)
                    .build();
            mContext.sendBroadcast(tileConfigurationIntent);
            mContext.startService(new Intent(mContext, TileServiceCompat.class));

        }
    }

    public static boolean installTile(Context context) {
        String qsTileId;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            qsTileId = "custom(com.topjohnwu.magisk/.services.TileServiceNewApi)";
        } else {
            qsTileId = "intent(Magisk)";
        }

        List<String> lines = Shell.su("settings get secure sysui_qs_tiles");
        if (lines != null && lines.size() == 1) {
            List<String> tiles = new LinkedList<>(Arrays.asList(lines.get(0).split(",")));
            Logger.dev("Utils: Current Tile String is " + tiles);
            if (tiles.size() > 1) {
                for (String tile : tiles) {
                    if (tile.equals(qsTileId)) {
                        return true;
                    }
                }

                tiles.add(Math.round(tiles.size() / 2), qsTileId);
                String newTiles = TextUtils.join(",", tiles);
                Logger.dev("Utils: NewtilesString is " + newTiles);
                Shell.su("settings put secure sysui_qs_tiles \"" + newTiles + "\"");
                if (Build.VERSION.SDK_INT == Build.VERSION_CODES.M) {
                    Utils.refreshService(context);
                }
                return true;
            }
        }
        return false;
    }

    public static boolean uninstallTile(Context context) {

        String qsTileId;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            qsTileId = "custom(com.topjohnwu.magisk/.services.TileServiceNewApi)";
        } else {
            qsTileId = "intent(Magisk)";
        }
        List<String> lines = Shell.su("settings get secure sysui_qs_tiles");
        if (lines != null && lines.size() == 1) {
            List<String> tiles = new LinkedList<>(Arrays.asList(lines.get(0).split(",")));
            if (tiles.size() > 1) {
                boolean isPresent = false;
                for (int i = 0; i < tiles.size(); i++) {
                    if (tiles.get(i).equals(qsTileId)) {
                        isPresent = true;
                        tiles.remove(i);
                        break;
                    }
                }
                if (isPresent) {
                    String newTiles = TextUtils.join(",", tiles);
                    Shell.su("settings put secure sysui_qs_tiles \"" + newTiles + "\"");
                    if (Build.VERSION.SDK_INT == Build.VERSION_CODES.M) {
                        Utils.refreshService(context);
                    }
                }
                return true;

            }
        }
        return false;
    }

    private static void refreshService(Context context) {
        context.startService(new Intent(context, TileServiceCompat.class));
    }

    // Gets an overall state for the quick settings tile
    // 0 for root disabled, 1 for root enabled (no auto), 2 for auto-root
    public static int CheckRootsState(Context mContext) {
        if (autoToggleEnabled(mContext)) {
            return 2;
        } else {
            if (rootEnabled()) {
                return 1;

            } else {
                return 0;

            }
        }
    }

    // To check if service is enabled
    public static boolean hasServicePermission(Context mContext) {
        int accessibilityEnabled = 0;
        final String service = mContext.getPackageName() + "/" + MonitorService.class.getCanonicalName();
        try {
            accessibilityEnabled = Settings.Secure.getInt(
                    mContext.getApplicationContext().getContentResolver(),
                    android.provider.Settings.Secure.ACCESSIBILITY_ENABLED);
        } catch (Settings.SettingNotFoundException e) {
            Log.e(TAG, "Error finding setting, default accessibility to not found: "
                    + e.getMessage());
        }
        TextUtils.SimpleStringSplitter mStringColonSplitter = new TextUtils.SimpleStringSplitter(':');

        if (accessibilityEnabled == 1) {
            String settingValue = Settings.Secure.getString(
                    mContext.getApplicationContext().getContentResolver(),
                    Settings.Secure.ENABLED_ACCESSIBILITY_SERVICES);
            if (settingValue != null) {
                mStringColonSplitter.setString(settingValue);
                while (mStringColonSplitter.hasNext()) {
                    String accessibilityService = mStringColonSplitter.next();

                    if (accessibilityService.equalsIgnoreCase(service)) {
                        return true;
                    }
                }
            }
        } else {
            Log.v(TAG, "***ACCESSIBILITY IS DISABLED***");
        }

        return false;
    }

    public static boolean isMyServiceRunning(Class<?> serviceClass, Context context) {
        ActivityManager manager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClass.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }

    public interface ItemClickListener {

        void onItemClick(View view, int position);

    }

}