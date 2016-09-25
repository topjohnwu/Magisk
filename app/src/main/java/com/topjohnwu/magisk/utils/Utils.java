package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.service.quicksettings.TileService;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.kcoppock.broadcasttilesupport.BroadcastTileIntentBuilder;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.RootFragment;
import com.topjohnwu.magisk.module.BaseModule;
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
import java.util.Comparator;
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

    public static int magiskVersion, remoteMagiskVersion = -1, remoteAppVersion = -1;
    public static String magiskLink, magiskChangelog, appChangelog, appLink, phhLink, supersuLink;
    private static final String TAG = "Magisk";

    public static final String MAGISK_PATH = "/magisk";
    public static final String MAGISK_CACHE_PATH = "/cache/magisk";
    public static final String UPDATE_JSON = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updates/magisk_update.json";

    public static void init(Context context) {
        List<String> ret = Shell.sh("getprop magisk.version");
        if (ret.get(0).replaceAll("\\s", "").isEmpty()) {
            magiskVersion = -1;
        } else {
            magiskVersion = Integer.parseInt(ret.get(0));
        }
        String toolPath = context.getApplicationInfo().dataDir + "/busybox";
        Shell.su("PATH=$PATH:" + toolPath);
    }

    public static boolean itemExist(String path) {
        List<String> ret;
        String command = "if [ -e " + path + " ]; then echo true; else echo false; fi";
        if (Shell.rootAccess()) {
            ret = Shell.su(command);
        } else {
            ret = Shell.sh(command);
        }
        return Boolean.parseBoolean(ret.get(0));
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
        Logger.dh("Utils: AutoRootEnableCheck is " + preferences.getBoolean("autoRootEnable", false));
        return PreferenceManager.getDefaultSharedPreferences(context).getBoolean("autoRootEnable", false);

    }

    public static boolean createFile(String path) {
        String command = "touch " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo true; else echo false; fi";
        if (!Shell.rootAccess()) {
            return false;
        } else {
            return Boolean.parseBoolean(Shell.su(command).get(0));
        }
    }

    public static boolean removeFile(String path) {
        boolean check;
        String command = "rm -f " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo false; else echo true; fi";
        if (!Shell.rootAccess()) {
            return false;
        } else {
            try {
                check = Boolean.parseBoolean(Shell.su(command).get(0));
                return check;
            } catch (NullPointerException e) {
                Log.d("Magisk:", "SU error executing removeFile " + e);
                return false;
            }
        }
    }

    public static void toggleRoot(Boolean b, Context context) {
        if (Utils.magiskVersion != -1) {
            if (b) {
                Shell.su("ln -s $(getprop magisk.supath) /magisk/.core/bin", "setprop magisk.root 1");
            } else {
                Shell.su("rm -rf /magisk/.core/bin", "setprop magisk.root 0");
            }
            if (PreferenceManager.getDefaultSharedPreferences(context).getBoolean("enable_quicktile", false)) {
                SetupQuickSettingsTile(context);
            }
            PreferenceManager.getDefaultSharedPreferences(context).edit().putBoolean("root",b).apply();
        }
    }

    public static void toggleAutoRoot(Boolean b, Context context) {
        Logger.dh("Utils: toggleAutocalled for " + b );
        if (Utils.magiskVersion != -1) {
            if (!Utils.hasServicePermission(context)) {
                Intent intent = new Intent(android.provider.Settings.ACTION_ACCESSIBILITY_SETTINGS);
                Toast.makeText(context, "Please enable Magisk in accessibility for auto-toggle work.", Toast.LENGTH_LONG).show();
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(intent);
            } else {
                Logger.dh("Utils: toggleAuto checks passed, setting" + b );
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
            SetupQuickSettingsTile(context);
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

    public static void downloadAndReceive(Context context, DownloadReceiver receiver, String link, String file) {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(context, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
            return;
        }
        File downloadFile, dir = new File(Environment.getExternalStorageDirectory() + "/MagiskManager");
        downloadFile = new File(dir + "/" + file);
        if (!dir.exists()) {
            if (!dir.mkdirs()) {
                Toast.makeText(context, R.string.toast_error_makedir, Toast.LENGTH_LONG).show();
                return;
            }
        }
        if (downloadFile.exists()) {
            if (!downloadFile.delete()) {
                Toast.makeText(context, R.string.toast_error_removing_files, Toast.LENGTH_LONG).show();
                return;
            }
        }

        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        DownloadManager.Request request = new DownloadManager.Request(Uri.parse(link));
        request.setDestinationUri(Uri.fromFile(downloadFile));

        receiver.setDownloadID(downloadManager.enqueue(request));
        context.registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
    }

    public static String procFile(String value, Context context) {

        String cryptoPass = context.getResources().getString(R.string.pass);
        try {
            DESKeySpec keySpec = new DESKeySpec(cryptoPass.getBytes("UTF8"));
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
            SecretKey key = keyFactory.generateSecret(keySpec);

            byte[] encrypedPwdBytes = Base64.decode(value, Base64.DEFAULT);
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
        return value;
    }

    public static void SetupQuickSettingsTile(Context mContext) {
        Logger.dh("Utils: SetupQuickSettings called");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            Logger.dh("Utils: Starting N quick settings service");
            Intent serviceIntent = new Intent(mContext, TileServiceNewApi.class);
            mContext.startService(serviceIntent);

        }
        if (Build.VERSION.SDK_INT == Build.VERSION_CODES.M) {
            Logger.dh("Utils: Marshmallow build detected");
            String mLabelString;
            int mRootIcon = R.drawable.root_white;
            int mAutoRootIcon = R.drawable.ic_autoroot_white;
            int mRootDisabled = R.drawable.root_grey;
            int mRootsState = CheckRootsState(mContext);
            Logger.dh("Utils: Root State returned as " + mRootsState);
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

    public static void installTile(Context context) {
        String qsTileId;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            qsTileId = "custom(com.topjohnwu.magisk/.services.TileServiceNewApi)";
        } else {
            qsTileId = "intent(Magisk)";
        }

        List<String> lines = Shell.su("settings get secure sysui_qs_tiles");
        if (lines != null && lines.size() == 1) {
            List<String> tiles = new LinkedList<>(Arrays.asList(lines.get(0).split(",")));
            Logger.dh("Utils: Current Tile String is " + tiles);
            if (tiles.size() > 1) {
                for (String tile : tiles) {
                    if (tile.equals(qsTileId)) {
                        Toast.makeText(context, "Tile already installed", Toast.LENGTH_SHORT).show();
                        return;
                    }
                }

                tiles.add(Math.round(tiles.size() / 2), qsTileId);
                String newTiles = TextUtils.join(",", tiles);
                Logger.dh("Utils: NewtilesString is " + newTiles);
                Shell.su("settings put secure sysui_qs_tiles \"" + newTiles + "\"");
                Toast.makeText(context, "Tile installed", Toast.LENGTH_SHORT).show();
                if (Build.VERSION.SDK_INT == Build.VERSION_CODES.M) {
                    Utils.refreshService(context);
                }
                return;
            }
        }
        Toast.makeText(context, "Tile installation error", Toast.LENGTH_SHORT).show();
    }

    public static void uninstallTile(Context context) {

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
                    Toast.makeText(context, "Tile uninstalled", Toast.LENGTH_SHORT).show();
                    if (Build.VERSION.SDK_INT == Build.VERSION_CODES.M) {
                        Utils.refreshService(context);
                    }
                    return;
                }
                Toast.makeText(context, "Tile already uninstalled", Toast.LENGTH_SHORT).show();

            }
        }
        Toast.makeText(context, "Tile uninstallation error", Toast.LENGTH_SHORT).show();
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

    public abstract static class DownloadReceiver extends BroadcastReceiver {
        public Context mContext;
        long downloadID;
        public String mName;

        public DownloadReceiver() {
        }

        public DownloadReceiver(String name) {
            mName = name;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            mContext = context;
            DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
            String action = intent.getAction();
            if (DownloadManager.ACTION_DOWNLOAD_COMPLETE.equals(action)) {
                DownloadManager.Query query = new DownloadManager.Query();
                query.setFilterById(downloadID);
                Cursor c = downloadManager.query(query);
                if (c.moveToFirst()) {
                    int columnIndex = c.getColumnIndex(DownloadManager.COLUMN_STATUS);
                    int status = c.getInt(columnIndex);
                    switch (status) {
                        case DownloadManager.STATUS_SUCCESSFUL:
                            File file = new File(Uri.parse(c.getString(c.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI))).getPath());
                            task(file);
                            break;
                        default:
                            Toast.makeText(context, R.string.download_file_error, Toast.LENGTH_LONG).show();
                            break;
                    }
                    context.unregisterReceiver(this);
                }
            }
        }

        public void setDownloadID(long id) {
            downloadID = id;
        }

        public abstract void task(File file);
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

    public static class ModuleComparator implements Comparator<BaseModule> {
        @Override
        public int compare(BaseModule o1, BaseModule o2) {
            return o1.getName().compareToIgnoreCase(o2.getName());
        }
    }
}