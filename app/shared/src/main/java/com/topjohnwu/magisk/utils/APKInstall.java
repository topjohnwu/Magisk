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
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public final class APKInstall {

    private static final String ACTION_SESSION_UPDATE = "ACTION_SESSION_UPDATE";

    // @WorkerThread
    public static void install(Context context, File apk) {
        try (var src = new FileInputStream(apk);
             var out = openStream(context, true)) {
            if (out != null)
                transfer(src, out);
        } catch (IOException e) {
            Log.e(APKInstall.class.getSimpleName(), "", e);
        }
    }

    public static OutputStream openStream(Context context, boolean silent) {
        //noinspection InlinedApi
        var flag = PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_MUTABLE;
        var intent = new Intent(ACTION_SESSION_UPDATE).setPackage(context.getPackageName());
        var pending = PendingIntent.getBroadcast(context, 0, intent, flag);

        var installer = context.getPackageManager().getPackageInstaller();
        var params = new SessionParams(SessionParams.MODE_FULL_INSTALL);
        if (silent && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            params.setRequireUserAction(SessionParams.USER_ACTION_NOT_REQUIRED);
        }
        try {
            Session session = installer.openSession(installer.createSession(params));
            var out = session.openWrite(UUID.randomUUID().toString(), 0, -1);
            return new FilterOutputStream(out) {
                @Override
                public void write(byte[] b, int off, int len) throws IOException {
                    out.write(b, off, len);
                }
                @Override
                public void close() throws IOException {
                    super.close();
                    session.commit(pending.getIntentSender());
                    session.close();
                }
            };
        } catch (IOException e) {
            Log.e(APKInstall.class.getSimpleName(), "", e);
        }
        return null;
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
        var receiver = new InstallReceiver(packageName, onSuccess);
        var filter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
        filter.addDataScheme("package");
        context.registerReceiver(receiver, filter);
        context.registerReceiver(receiver, new IntentFilter(ACTION_SESSION_UPDATE));
        return receiver;
    }

    public static class InstallReceiver extends BroadcastReceiver {
        private final String packageName;
        private final Runnable onSuccess;
        private final CountDownLatch latch = new CountDownLatch(1);
        private Intent userAction = null;

        private InstallReceiver(String packageName, Runnable onSuccess) {
            this.packageName = packageName;
            this.onSuccess = onSuccess;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (Intent.ACTION_PACKAGE_ADDED.equals(intent.getAction())) {
                Uri data = intent.getData();
                if (data == null)
                    return;
                String pkg = data.getSchemeSpecificPart();
                if (pkg.equals(packageName)) {
                    if (onSuccess != null)
                        onSuccess.run();
                    context.unregisterReceiver(this);
                }
                return;
            }
            int status = intent.getIntExtra(EXTRA_STATUS, STATUS_FAILURE_INVALID);
            switch (status) {
                case STATUS_PENDING_USER_ACTION:
                    userAction = intent.getParcelableExtra(Intent.EXTRA_INTENT);
                    break;
                case STATUS_SUCCESS:
                    if (onSuccess != null)
                        onSuccess.run();
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
            } catch (Exception ignored) {}
            return userAction;
        }
    }
}
