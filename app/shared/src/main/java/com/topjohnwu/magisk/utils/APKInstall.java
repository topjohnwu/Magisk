package com.topjohnwu.magisk.utils;

import static android.content.pm.PackageInstaller.EXTRA_STATUS;
import static android.content.pm.PackageInstaller.STATUS_FAILURE_INVALID;
import static android.content.pm.PackageInstaller.STATUS_PENDING_USER_ACTION;
import static android.content.pm.PackageInstaller.STATUS_SUCCESS;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInstaller.Session;
import android.content.pm.PackageInstaller.SessionParams;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public final class APKInstall {
    // @WorkerThread
    public static void installapk(Context context, File apk) {
        //noinspection InlinedApi
        var flag = PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_MUTABLE;
        var action = APKInstall.class.getName();
        var intent = new Intent(action).setPackage(context.getPackageName());
        var pending = PendingIntent.getBroadcast(context, 0, intent, flag);

        var installer = context.getPackageManager().getPackageInstaller();
        var params = new SessionParams(SessionParams.MODE_FULL_INSTALL);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            params.setRequireUserAction(SessionParams.USER_ACTION_NOT_REQUIRED);
        }
        try (Session session = installer.openSession(installer.createSession(params))) {
            OutputStream out = session.openWrite(apk.getName(), 0, apk.length());
            try (var in = new FileInputStream(apk); out) {
                transfer(in, out);
            }
            session.commit(pending.getIntentSender());
        } catch (IOException e) {
            Log.e(APKInstall.class.getSimpleName(), "", e);
        }
    }

    public static void transfer(InputStream in, OutputStream out) throws IOException {
        int size = 8192;
        var buffer = new byte[size];
        int read;
        while ((read = in.read(buffer, 0, size)) >= 0) {
            out.write(buffer, 0, read);
        }
    }

    public static InstallReceiver register(Context context, String packageName, Runnable onSuccess) {
        var receiver = new InstallReceiver(context, packageName, onSuccess);
        var filter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
        filter.addDataScheme("package");
        context.registerReceiver(receiver, filter);
        context.registerReceiver(receiver, new IntentFilter(APKInstall.class.getName()));
        return receiver;
    }

    public static class InstallReceiver extends BroadcastReceiver {
        private final Context context;
        private final String packageName;
        private final Runnable onSuccess;
        private final CountDownLatch latch = new CountDownLatch(1);
        private Intent intent = null;

        private InstallReceiver(Context context, String packageName, Runnable onSuccess) {
            this.context = context;
            this.packageName = packageName;
            this.onSuccess = onSuccess;
        }

        @Override
        public void onReceive(Context c, Intent i) {
            if (Intent.ACTION_PACKAGE_ADDED.equals(i.getAction())) {
                Uri data = i.getData();
                if (data == null || onSuccess == null) return;
                String pkg = data.getSchemeSpecificPart();
                if (pkg.equals(packageName)) {
                    onSuccess.run();
                    context.unregisterReceiver(this);
                }
                return;
            }
            int status = i.getIntExtra(EXTRA_STATUS, STATUS_FAILURE_INVALID);
            switch (status) {
                case STATUS_PENDING_USER_ACTION:
                    intent = i.getParcelableExtra(Intent.EXTRA_INTENT);
                    break;
                case STATUS_SUCCESS:
                    if (onSuccess != null) onSuccess.run();
                default:
                    context.unregisterReceiver(this);
            }
            latch.countDown();
        }

        // @WorkerThread @Nullable
        public Intent waitIntent() {
            try {
                //noinspection ResultOfMethodCallIgnored
                latch.await(5, TimeUnit.SECONDS);
            } catch (Exception ignored) {
            }
            return intent;
        }
    }
}
