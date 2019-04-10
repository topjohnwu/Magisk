package com.topjohnwu.magisk.components;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;

import androidx.annotation.Nullable;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.uicomponents.Notifications;
import com.topjohnwu.magisk.uicomponents.ProgressNotification;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class DownloadModuleService extends Service {

    private List<ProgressNotification> notifications;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        notifications = new ArrayList<>();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Shell.EXECUTOR.execute(() -> {
            Repo repo = intent.getParcelableExtra("repo");
            boolean install = intent.getBooleanExtra("install", false);
            dlProcessInstall(repo, install);
        });
        return START_REDELIVER_INTENT;
    }

    @Override
    public synchronized void onTaskRemoved(Intent rootIntent) {
        for (ProgressNotification n : notifications) {
            Notifications.mgr.cancel(n.hashCode());
        }
        notifications.clear();
    }

    private synchronized void addNotification(ProgressNotification n) {
        if (notifications.isEmpty()) {
            // Start foreground
            startForeground(n.hashCode(), n.getNotification());
        }
        notifications.add(n);
    }

    private synchronized void removeNotification(ProgressNotification n) {
        notifications.remove(n);
        if (notifications.isEmpty()) {
            // No more tasks, stop service
            stopForeground(true);
            stopSelf();
        } else {
            // Pick another notification as our foreground notification
            n = notifications.get(0);
            startForeground(n.hashCode(), n.getNotification());
        }
    }

    private void dlProcessInstall(Repo repo, boolean install) {
        File output = new File(Const.EXTERNAL_PATH, repo.getDownloadFilename());
        ProgressNotification progress = new ProgressNotification(output.getName());
        addNotification(progress);
        try {
            InputStream in = Networking.get(repo.getZipUrl())
                    .setDownloadProgressListener(progress)
                    .execForInputStream().getResult();
            OutputStream out = new BufferedOutputStream(new FileOutputStream(output));
            processZip(in, out, repo.isNewInstaller());
            Intent intent = new Intent(this, ClassMap.get(FlashActivity.class));
            intent.setData(Uri.fromFile(output))
                    .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
            synchronized (getApplication()) {
                if (install && App.foreground() != null &&
                        !(App.foreground() instanceof FlashActivity)) {
                    /* Only start flashing if there is a foreground activity and the
                     * user is not also flashing another module at the same time */
                    App.foreground().startActivity(intent);
                } else {
                    /* Or else we preset a notification notifying that we are done */
                    PendingIntent pi = PendingIntent.getActivity(this, progress.hashCode(), intent,
                            PendingIntent.FLAG_UPDATE_CURRENT);
                    progress.dlDone(pi);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            progress.dlFail();
        }
        removeNotification(progress);
    }

    private void processZip(InputStream in, OutputStream out, boolean inject)
            throws IOException {
        try (ZipInputStream zin = new ZipInputStream(in);
             ZipOutputStream zout = new ZipOutputStream(out)) {

            if (inject) {
                // Inject latest module-installer.sh as update-binary
                zout.putNextEntry(new ZipEntry("META-INF/"));
                zout.putNextEntry(new ZipEntry("META-INF/com/"));
                zout.putNextEntry(new ZipEntry("META-INF/com/google/"));
                zout.putNextEntry(new ZipEntry("META-INF/com/google/android/"));
                zout.putNextEntry(new ZipEntry("META-INF/com/google/android/update-binary"));
                try (InputStream update_bin = Networking.get(Const.Url.MODULE_INSTALLER)
                        .execForInputStream().getResult()) {
                    ShellUtils.pump(update_bin, zout);
                }
                zout.putNextEntry(new ZipEntry("META-INF/com/google/android/updater-script"));
                zout.write("#MAGISK\n".getBytes("UTF-8"));
            }

            int off = -1;
            ZipEntry entry;
            while ((entry = zin.getNextEntry()) != null) {
                if (off < 0)
                    off = entry.getName().indexOf('/') + 1;
                String path = entry.getName().substring(off);
                if (path.isEmpty())
                    continue;
                if (inject && path.startsWith("META-INF"))
                    continue;
                zout.putNextEntry(new ZipEntry(path));
                if (!entry.isDirectory())
                    ShellUtils.pump(zin, zout);
            }
        }
    }
}
