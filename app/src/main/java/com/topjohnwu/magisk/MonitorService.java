package com.topjohnwu.magisk;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.view.accessibility.AccessibilityEvent;

import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.Set;

public class MonitorService extends AccessibilityService {
    private static final String TAG = "Magisk";
    private final Handler handler = new Handler();
    private Boolean disableroot;
    private Boolean disablerootprev;
    private int counter = 0;
    private String mPackageName = "";

    @Override
    protected void onServiceConnected() {
        super.onServiceConnected();

        //Configure these here for compatibility with API 13 and below.
        AccessibilityServiceInfo config = new AccessibilityServiceInfo();
        config.eventTypes = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED;
        config.feedbackType = AccessibilityServiceInfo.FEEDBACK_GENERIC;
        disableroot = false;
        disablerootprev = disableroot;
        if (Build.VERSION.SDK_INT >= 16)
            //Just in case this helps
            config.flags = AccessibilityServiceInfo.FLAG_INCLUDE_NOT_IMPORTANT_VIEWS;

        setServiceInfo(config);

    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("Magisk","MonitorService: Service created");
    }

    @Override
    public void onAccessibilityEvent(AccessibilityEvent event) {
        if (event.getEventType() == AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED) {
            ComponentName componentName = new ComponentName(
                    event.getPackageName().toString(),
                    event.getClassName().toString()
            );

            ActivityInfo activityInfo = tryGetActivity(componentName);
            boolean isActivity = activityInfo != null;
            if (isActivity) {
                Log.i("Magisk","CurrentActivity: " + componentName.getPackageName());
                String mPackage = componentName.getPackageName();

                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
                if (prefs.getBoolean("autoRootEnable", false)) {

                    Set<String> setBlackList = prefs.getStringSet("auto_blacklist", null);
                    Set<String> setWhiteList = prefs.getStringSet("auto_whitelist", null);

                    if (setBlackList != null) {
                        disableroot = setBlackList.contains(mPackage);
                        if (disableroot) {
                            ForceDisableRoot();
                        } else {
                            ForceEnableRoot();
                        }
                        String appFriendly = getAppName(mPackage);
                        ShowNotification(disableroot,appFriendly);
                    }
                }
            }
        }
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
            return "";
        }
    }

    private ActivityInfo tryGetActivity(ComponentName componentName) {
        try {
            return getPackageManager().getActivityInfo(componentName, 0);
        } catch (PackageManager.NameNotFoundException e) {
            return null;
        }
    }

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

    private void ShowNotification(boolean rootAction, String packageName) {
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
            if (packageName.equals("")) {
                rootMessage = "Root has been disabled";
            } else {
                rootMessage = "Root has been disabled for " + packageName;
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

    @Override
    public void onInterrupt() {
    }
}