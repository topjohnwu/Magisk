package com.topjohnwu.magisk;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.usage.UsageStats;
import android.app.usage.UsageStatsManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
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
    private String mPackageName;

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

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("Magisk","MonitorService: Service created");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d("Magisk","MonitorService: Service destroyed");
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
        Utils.toggleRoot(false);
        if (Utils.rootEnabled()) {
            Utils.toggleRoot(false);
            Log.d(TAG, "MonitorService: FORCING.");
        }
        Log.d("Magisk", "MonitorService: Forcedisable called. " + Utils.rootEnabled());
    }

    private void ForceEnableRoot() {
        Log.d("Magisk", "MonitorService: ForceEnable called.");
        Utils.toggleRoot(true);
        if (!Utils.rootEnabled()) {
            Utils.toggleRoot(true);
            }
    }

    private void ShowNotification(boolean rootAction) {
        NotificationManager mNotifyMgr = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        NotificationCompat.Builder mBuilder;

        if (rootAction) {

            Intent intent = new Intent(getApplication(), WelcomeActivity.class);
            intent.putExtra("relaunch", "relaunch");
            String rootMessage;
            PendingIntent pendingIntent = PendingIntent.getActivity(
                    getApplicationContext(),
                    0,
                    intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            if (mPackageName.equals("")) {
                rootMessage = "Root has been disabled";
            } else {
                rootMessage = "Root has been disabled for " + mPackageName;
            }
            mBuilder =
                    new NotificationCompat.Builder(getApplicationContext())
                            .setSmallIcon(disableroot ? R.drawable.ic_stat_notification_autoroot_off : R.drawable.ic_stat_notification_autoroot_on)
                            .setContentIntent(pendingIntent)
                            .setContentTitle("Auto-root status changed")
                            .setContentText(rootMessage);
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

    private String getAppName (String packageName) {
        PackageManager pkManager = getPackageManager();
        ApplicationInfo appInfo;
        String appname = "";
        try {
            appInfo = pkManager.getApplicationInfo(packageName, 0);
            appname = (String) ((appInfo != null) ? pkManager.getApplicationLabel(appInfo) : "???");
            return appname;
        } catch (final PackageManager.NameNotFoundException e) {
            return null;
        }
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
                    mPackageName = getAppName(topPackageName);
                }

            }
        }

        return topPackageName.equals(packageName);
    }

}

