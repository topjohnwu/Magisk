package com.topjohnwu.magisk;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.usage.UsageStats;
import android.app.usage.UsageStatsManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;

public class MonitorService extends Service

{

    private static final String TAG = "Magisk";
    private final Handler handler = new Handler();
    private Boolean disableroot;
    private Boolean disablerootprev;
    private int counter = 0;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        checkProcesses.run();

        return START_STICKY;

    }

    private Runnable checkProcesses = new Runnable() {
        @Override
        public void run() {

            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
            if (prefs.getBoolean("autoRootEnable", false)) {

                Set<String> setBlackList = prefs.getStringSet("auto_blacklist", null);
                Set<String> setWhiteList = prefs.getStringSet("auto_whitelist", null);

                if (setBlackList != null) {
                    disableroot = getStats(setBlackList);
                }

                if (disableroot != disablerootprev) {

                    String rootstatus = (disableroot ? "disabled" : "enabled");
                    if (disableroot) {
                        ForceDisableRoot();
                    } else {
                        ForceEnableRoot();
                    }

                    ShowNotification(disableroot);

                }
                disablerootprev = disableroot;
                //Log.d(TAG,"Root check completed, set to " + (disableroot ? "disabled" : "enabled"));

            }
            handler.postDelayed(checkProcesses, 1000);
        }

    };

    private void ForceDisableRoot() {
        Log.d("Magisk", "MonitorService: Forcedisable called.");
        Shell.su("setprop magisk.root 0");

        if (Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 0"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 0"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 0"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 0"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 0"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 0"));
            Log.d(TAG, "MonitorService: FORCING.");
        }
        Log.d("Magisk", "MonitorService: Forcedisable called. " + Utils.rootStatus());
    }

    private void ForceEnableRoot() {
        Log.d("Magisk", "MonitorService: ForceEnable called.");
        if (!Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 1"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (!Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 1"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (!Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 1"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (!Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 1"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (!Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 1"));
            Log.d(TAG, "MonitorService: FORCING.");
        } else if (!Utils.rootStatus()) {
            Shell.su(("setprop magisk.root 1"));
            Log.d(TAG, "MonitorService: FORCING.");
        }
    }

    private void ShowNotification(boolean rootAction) {
        NotificationManager mNotifyMgr = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        NotificationCompat.Builder mBuilder;

        if (rootAction) {

            Intent intent = new Intent(getApplication(), WelcomeActivity.class);
            intent.putExtra("relaunch", "relaunch");
            PendingIntent pendingIntent = PendingIntent.getActivity(
                    getApplicationContext(),
                    0,
                    intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);

            mBuilder =
                    new NotificationCompat.Builder(getApplicationContext())
                            .setSmallIcon(disableroot ? R.drawable.ic_stat_notification_autoroot_off : R.drawable.ic_stat_notification_autoroot_on)
                            .setContentIntent(pendingIntent)
                            .setContentTitle("Auto-root status changed")
                            .setContentText("Root has been disabled.");
            int mNotificationId = 1;
            mNotifyMgr.notify(mNotificationId, mBuilder.build());
        } else {
            mNotifyMgr.cancelAll();
        }

    }

    private boolean getStats(Set<String> seti) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            boolean inStats = false;
            if (seti != null) {
                ArrayList<String> statList = new ArrayList<>(seti);
                for (int i = 0; i < statList.size(); i++) {
                    if (isAppForeground(statList.get(i))) {
                        inStats = (isAppForeground(statList.get(i)));
                    }
                }
                return inStats;
            }
            Log.d(TAG, "SDK check failed.");
        }
        return false;
    }

    protected boolean isAppForeground(String packageName) {
        UsageStatsManager usageStatsManager = (UsageStatsManager) getSystemService(Context.USAGE_STATS_SERVICE);
        long time = System.currentTimeMillis();
        List<UsageStats> stats = usageStatsManager.queryUsageStats(UsageStatsManager.INTERVAL_DAILY, time - 1000 * 10, time);
        String topPackageName = "";
        if (stats != null) {

            SortedMap<Long, UsageStats> mySortedMap = new TreeMap<>();
            for (UsageStats usageStats : stats) {
                    mySortedMap.put(usageStats.getLastTimeUsed(), usageStats);

            }
            if (!mySortedMap.isEmpty()) {
                topPackageName = mySortedMap.get(mySortedMap.lastKey()).getPackageName();
                if (topPackageName.equals("com.topjohnwu.magisk")) {
                    mySortedMap.remove(mySortedMap.lastKey());
                    topPackageName = mySortedMap.get(mySortedMap.lastKey()).getPackageName();
                }
                //Log.d("Magisk", "MonitorService: Hi Captain, the package we need to kill for is " + topPackageName);
            }
        }

        return topPackageName.equals(packageName);
    }

}

