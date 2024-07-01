package com.topjohnwu.magisk.utils;

import static android.content.pm.PackageInstaller.EXTRA_SESSION_ID;
import static android.content.pm.PackageInstaller.EXTRA_STATUS;
import static android.content.pm.PackageInstaller.STATUS_FAILURE_INVALID;
import static android.content.pm.PackageInstaller.STATUS_PENDING_USER_ACTION;
import static android.content.pm.PackageInstaller.STATUS_SUCCESS;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInstaller.SessionParams;
import android.net.Uri;
import android.os.Build;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public final class APKInstall {

    public static void transfer(InputStream in, OutputStream out) throws IOException {
        int size = 8192;
        var buffer = new byte[size];
        int read;
        while ((read = in.read(buffer, 0, size)) >= 0) {
            out.write(buffer, 0, read);
        }
    }

    public static void registerReceiver(
            Context context, BroadcastReceiver receiver, IntentFilter filter) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            // noinspection InlinedApi
            context.registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            context.registerReceiver(receiver, filter);
        }
    }

    public static Session startSession(Context context) {
        return startSession(context, null, null, null);
    }

    public static Session startSession(Context context, String pkg,
                                       Runnable onFailure, Runnable onSuccess) {
        var receiver = new InstallReceiver(pkg, onSuccess, onFailure);
        context = context.getApplicationContext();
        if (pkg != null) {
            // If pkg is not null, look for package added event
            var filter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
            filter.addDataScheme("package");
            registerReceiver(context, receiver, filter);
        }
        registerReceiver(context, receiver, new IntentFilter(receiver.sessionId));
        return receiver;
    }

    public interface Session {
        // @WorkerThread
        OutputStream openStream(Context context) throws IOException;
        // @WorkerThread @Nullable
        Intent waitIntent();
    }

    private static class InstallReceiver extends BroadcastReceiver implements Session {
        private final String packageName;
        private final Runnable onSuccess;
        private final Runnable onFailure;
        private final CountDownLatch latch = new CountDownLatch(1);
        private Intent userAction = null;

        final String sessionId = UUID.randomUUID().toString();

        private InstallReceiver(String packageName, Runnable onSuccess, Runnable onFailure) {
            this.packageName = packageName;
            this.onSuccess = onSuccess;
            this.onFailure = onFailure;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (Intent.ACTION_PACKAGE_ADDED.equals(intent.getAction())) {
                Uri data = intent.getData();
                if (data == null)
                    return;
                String pkg = data.getSchemeSpecificPart();
                if (pkg.equals(packageName)) {
                    onSuccess(context);
                }
            } else if (sessionId.equals(intent.getAction())) {
                int status = intent.getIntExtra(EXTRA_STATUS, STATUS_FAILURE_INVALID);
                switch (status) {
                    case STATUS_PENDING_USER_ACTION ->
                            userAction = intent.getParcelableExtra(Intent.EXTRA_INTENT);
                    case STATUS_SUCCESS -> {
                        if (packageName == null) {
                            onSuccess(context);
                        }
                    }
                    default -> {
                        int id = intent.getIntExtra(EXTRA_SESSION_ID, 0);
                        var installer = context.getPackageManager().getPackageInstaller();
                        try {
                            installer.abandonSession(id);
                        } catch (SecurityException ignored) {
                        }
                        if (onFailure != null) {
                            onFailure.run();
                        }
                        try {
                            context.getApplicationContext().unregisterReceiver(this);
                        } catch (IllegalArgumentException ignored) {
                        }
                    }
                }
                latch.countDown();
            }
        }

        private void onSuccess(Context context) {
            if (onSuccess != null)
                onSuccess.run();
            try {
                context.getApplicationContext().unregisterReceiver(this);
            } catch (IllegalArgumentException ignored) {
            }
        }

        @Override
        public Intent waitIntent() {
            try {
                // noinspection ResultOfMethodCallIgnored
                latch.await(5, TimeUnit.SECONDS);
            } catch (Exception ignored) {}
            return userAction;
        }

        @Override
        public OutputStream openStream(Context context) throws IOException {
            // noinspection InlinedApi
            var flag = PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_MUTABLE;
            var intent = new Intent(sessionId).setPackage(context.getPackageName());
            var pending = PendingIntent.getBroadcast(context, 0, intent, flag);

            var installer = context.getPackageManager().getPackageInstaller();
            var params = new SessionParams(SessionParams.MODE_FULL_INSTALL);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                params.setRequireUserAction(SessionParams.USER_ACTION_NOT_REQUIRED);
            }
            var session = installer.openSession(installer.createSession(params));
            var out = session.openWrite(sessionId, 0, -1);
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
        }
    }
}
