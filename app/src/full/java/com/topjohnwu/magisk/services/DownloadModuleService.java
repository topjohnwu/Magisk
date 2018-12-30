package com.topjohnwu.magisk.services;

import android.app.Service;
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.widget.Toast;

import com.topjohnwu.core.Const;
import com.topjohnwu.core.container.Repo;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.ProgressNotification;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import androidx.annotation.Nullable;

public class DownloadModuleService extends Service {

    private boolean running = false;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (flags == 0 && running) {
            Utils.toast(R.string.dl_one_module, Toast.LENGTH_LONG);
        } else {
            running = true;
            Shell.EXECUTOR.execute(() -> {
                Repo repo = intent.getParcelableExtra("repo");
                boolean install = intent.getBooleanExtra("install", false);
                dlProcessInstall(repo, install);
                stopSelf();
            });
        }
        return START_REDELIVER_INTENT;
    }

    private void dlProcessInstall(Repo repo, boolean install) {
        File output = new File(Const.EXTERNAL_PATH, repo.getDownloadFilename());
        ProgressNotification progress = new ProgressNotification(output.getName());
        startForeground(progress.hashCode(), progress.getNotification());
        try {
            InputStream in = Networking.get(repo.getZipUrl())
                    .setDownloadProgressListener(progress)
                    .execForInputStream().getResult();
            removeTopFolder(in, new BufferedOutputStream(new FileOutputStream(output)));
            if (install) {
                progress.dismiss();
                Intent intent = new Intent(this, ClassMap.get(FlashActivity.class));
                intent.setData(Uri.fromFile(output))
                        .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                        .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
                startActivity(intent);
            } else {
                progress.dlDone();
            }
        } catch (Exception e) {
            e.printStackTrace();
            progress.dlFail();
        }
    }

    private void removeTopFolder(InputStream in, OutputStream out) throws IOException {
        try (ZipInputStream zin = new ZipInputStream(in);
             ZipOutputStream zout = new ZipOutputStream(out)) {
            ZipEntry entry;
            int off = -1;
            while ((entry = zin.getNextEntry()) != null) {
                if (off < 0)
                    off = entry.getName().indexOf('/') + 1;
                String path = entry.getName().substring(off);
                if (path.isEmpty())
                    continue;
                zout.putNextEntry(new ZipEntry(path));
                if (!entry.isDirectory())
                    ShellUtils.pump(zin, zout);
            }
        }
    }
}
