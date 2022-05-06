package com.topjohnwu.magisk;

import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.os.Build;
import android.os.Handler;

import androidx.annotation.NonNull;

import com.microsoft.appcenter.AppCenter;
import com.microsoft.appcenter.analytics.Analytics;
import com.microsoft.appcenter.channel.AbstractChannelListener;
import com.microsoft.appcenter.channel.Channel;
import com.microsoft.appcenter.crashes.AbstractCrashesListener;
import com.microsoft.appcenter.crashes.Crashes;
import com.microsoft.appcenter.crashes.ingestion.models.ErrorAttachmentLog;
import com.microsoft.appcenter.crashes.model.ErrorReport;
import com.microsoft.appcenter.ingestion.models.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Map;

public class Telemetry {
    private static final Channel.Listener patchDeviceListener = new AbstractChannelListener() {
        @Override
        public void onPreparedLog(@NonNull Log log, @NonNull String groupName, int flags) {
            var device = log.getDevice();
            device.setAppVersion(BuildConfig.VERSION_NAME);
            device.setAppBuild(String.valueOf(BuildConfig.VERSION_CODE));
        }
    };

    private static void addPatchDeviceListener() {
        try {
            var channelField = AppCenter.class.getDeclaredField("mChannel");
            channelField.setAccessible(true);
            var channel = (Channel) channelField.get(AppCenter.getInstance());
            assert channel != null;
            channel.addListener(patchDeviceListener);
        } catch (ReflectiveOperationException ignored) {
        }
    }

    private static void patchDevice() {
        try {
            var handlerField = AppCenter.class.getDeclaredField("mHandler");
            handlerField.setAccessible(true);
            var handler = ((Handler) handlerField.get(AppCenter.getInstance()));
            assert handler != null;
            handler.post(Telemetry::addPatchDeviceListener);
        } catch (ReflectiveOperationException ignored) {
        }
    }

    private static Application getDeApplication(Application app) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) return app;
        var deApp = new Application() {
            @Override
            public void registerActivityLifecycleCallbacks(ActivityLifecycleCallbacks callback) {
                app.registerActivityLifecycleCallbacks(callback);
            }
        };
        try {
            var attach = ContextWrapper.class.getDeclaredMethod("attachBaseContext", Context.class);
            attach.setAccessible(true);
            attach.invoke(deApp, app.getApplicationContext().createDeviceProtectedStorageContext());
            return deApp;
        } catch (ReflectiveOperationException ignored) {
            return app;
        }
    }

    public static void start(Application app, String text, String fileName) {
        Crashes.setListener(new AbstractCrashesListener() {
            @Override
            public Iterable<ErrorAttachmentLog> getErrorAttachments(ErrorReport report) {
                var list = new ArrayList<ErrorAttachmentLog>(2);
                list.add(ErrorAttachmentLog.attachmentWithText(text, fileName));
                String t;
                try {
                    Process process = Runtime.getRuntime().exec("logcat -d");
                    InputStream inputStream = process.getInputStream();
                    ByteArrayOutputStream result = new ByteArrayOutputStream();
                    byte[] buffer = new byte[1024];
                    for (int length; (length = inputStream.read(buffer)) != -1; ) {
                        result.write(buffer, 0, length);
                    }
                    t = result.toString();
                } catch (IOException e) {
                    t = android.util.Log.getStackTraceString(e);
                }
                list.add(ErrorAttachmentLog.attachmentWithText(t, "log.txt"));
                return list;
            }
        });
        AppCenter.start(getDeApplication(app), "7dd0af85-8cce-4a90-8792-f1d10ef34f67",
                Analytics.class, Crashes.class);
        AppCenter.setLogLevel(android.util.Log.ERROR);
        patchDevice();
    }

    public static void trackEvent(String name, Map<String, String> properties) {
        Analytics.trackEvent(name, properties);
    }

    public static void trackError(Throwable throwable, Map<String, String> properties) {
        Crashes.trackError(throwable, properties, null);
    }

    public static void trackError(Throwable throwable, String text) {
        var list = new ArrayList<ErrorAttachmentLog>(1);
        list.add(ErrorAttachmentLog.attachmentWithText(text, "log.txt"));
        Crashes.trackError(throwable, null, list);
    }
}
