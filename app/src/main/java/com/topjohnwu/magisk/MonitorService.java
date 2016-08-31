package com.topjohnwu.magisk;

import android.app.AppOpsManager;
import android.app.Fragment;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.usage.UsageStats;
import android.app.usage.UsageStatsManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.CountDownTimer;
import android.os.IBinder;
import android.os.Process;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.topjohnwu.magisk.utils.Shell;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;

public class MonitorService extends Service

{

    private static final String TAG = "Magisk";
    private UsageStatsManager mUsageStatsManager;
    private SharedPreferences prefs;
    private Boolean disableroot;
    private Boolean disablerootprev;
    public static boolean isRecursionEnable = false;
    private NotificationManager mNotifyMgr;
    private CountDownTimer timer = new CountDownTimer(1000, 1000) {

        @Override
        public void onTick(long millisUntilFinished) {

        }

        @Override
        public void onFinish() {
            CheckProcesses();

        }
    }.start();

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (!intent.hasExtra("disable")) {


            new Thread(new Runnable() {
                @Override
                public void run() {
                    // DO your work here
                    // get the data
                    timer.start();
                }
            }).start();


            return START_STICKY;
        } else return STOP_FOREGROUND_REMOVE;
    }


    @Override
    public void onDestroy() {
        Log.d(TAG, "Destroyah!");
        android.os.Process.killProcess(android.os.Process.myPid());
        super.onDestroy();
    }

    private boolean getStats(Set<String> seti) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            boolean inStats = false;
            UsageStatsManager lUsageStatsManager = (UsageStatsManager) getSystemService(Context.USAGE_STATS_SERVICE);
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

        String topPackageName = new String();

        if (stats != null) {
            SortedMap<Long, UsageStats> mySortedMap = new TreeMap<>();

            for (UsageStats usageStats : stats) {
                mySortedMap.put(usageStats.getLastTimeUsed(), usageStats);
            }

            if (mySortedMap != null && !mySortedMap.isEmpty()) {
                topPackageName = mySortedMap.get(mySortedMap.lastKey()).getPackageName();
            }
        }

        return topPackageName.equals(packageName);
    }

    private void CheckProcesses() {
        prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        if (prefs.getBoolean("autoRootEnable", false)) {

            Set<String> set = prefs.getStringSet("autoapps", null);

            ArrayList<String> arrayList = null;
            if (set != null) {
                disableroot = getStats(set);

            }
            if (disableroot != disablerootprev) {
                String rootstatus = (disableroot ? "disabled" : "enabled");
                Shell.su((disableroot ? "setprop magisk.root 0" : "setprop magisk.root 1"));
                mNotifyMgr = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
                NotificationCompat.Builder mBuilder;
                mNotifyMgr.cancelAll();
                if (disableroot) {
                    timer.cancel();
                    this.stopSelf();
                    Intent intent = new Intent(this, WelcomeActivity.class);
                    intent.putExtra("relaunch", "relaunch");
                    PendingIntent pendingIntent = PendingIntent.getActivity(
                            this,
                            0,
                            intent,
                            PendingIntent.FLAG_UPDATE_CURRENT);

                    mBuilder =
                            new NotificationCompat.Builder(this)
                                    .setSmallIcon(disableroot ? R.drawable.ic_stat_notification_autoroot_off : R.drawable.ic_stat_notification_autoroot_on)
                                    .setContentIntent(pendingIntent)
                                    .setContentTitle("Auto-root status changed")
                                    .setContentText("Auto root has been " + rootstatus + "!  Tap to re-enable when done.");

                } else {
                    mBuilder =
                            new NotificationCompat.Builder(this)
                                    .setAutoCancel(true)
                                    .setSmallIcon(disableroot ? R.drawable.ic_stat_notification_autoroot_off : R.drawable.ic_stat_notification_autoroot_on)
                                    .setContentTitle("Auto-root status changed")
                                    .setContentText("Auto root has been " + rootstatus + "!");
                }
// Builds the notification and issues it.
                int mNotificationId = 001;
                mNotifyMgr.notify(mNotificationId, mBuilder.build());

            }
            disablerootprev = disableroot;


            Log.d(TAG, "Check Processes finished, result is " + disableroot + " and settings say we should be " + prefs.getBoolean("autoRootEnable", false));
            timer.start();
        }
    }

    private boolean hasUsagePermission() {
        AppOpsManager appOps = (AppOpsManager)
                getSystemService(Context.APP_OPS_SERVICE);
        int mode = appOps.checkOpNoThrow(AppOpsManager.OPSTR_GET_USAGE_STATS,
                Process.myUid(), getPackageName());
        return mode == AppOpsManager.MODE_ALLOWED;
    }


}

