package com.topjohnwu.magisk.utils;

import android.os.AsyncTask;

import com.androidnetworking.AndroidNetworking;
import com.androidnetworking.error.ANError;
import com.androidnetworking.interfaces.DownloadListener;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.PatchAPK;
import com.topjohnwu.magisk.components.NotificationProgress;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.utils.JarMap;
import com.topjohnwu.utils.SignAPK;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;

public class DlInstallManager {

    public static void upgrade(String name) {
        dlInstall(name, new PatchPackageName());
    }

    public static void restore() {
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);
        dlInstall(name, new RestoreManager());
    }

    public static void dlInstall(String name, ManagerDownloadListener listener) {
        MagiskManager mm = Data.MM();
        File apk = new File(mm.getFilesDir(), "manager.apk");
        NotificationProgress progress = new NotificationProgress(name);
        listener.setInstances(apk, progress);
        AndroidNetworking
                .download(Data.managerLink, apk.getParent(), apk.getName())
                .setExecutor(AsyncTask.THREAD_POOL_EXECUTOR)
                .build()
                .setDownloadProgressListener(progress)
                .startDownload(listener);
    }

    public abstract static class ManagerDownloadListener implements DownloadListener {
        private File apk;
        private NotificationProgress progress;

        private void setInstances(File apk, NotificationProgress progress) {
            this.apk = apk;
            this.progress = progress;
        }

        public abstract void onDownloadComplete(File apk, NotificationProgress progress);

        @Override
        public final void onDownloadComplete() {
            onDownloadComplete(apk, progress);
        }

        @Override
        public void onError(ANError anError) {
            progress.dlFail();
        }
    }

    private static class PatchPackageName extends ManagerDownloadListener {

        @Override
        public void onDownloadComplete(File apk, NotificationProgress progress) {
            File patched = apk;
            MagiskManager mm = Data.MM();
            if (!mm.getPackageName().equals(BuildConfig.APPLICATION_ID)) {
                progress.getNotification()
                        .setProgress(0, 0, true)
                        .setContentTitle(mm.getString(R.string.hide_manager_title))
                        .setContentText("");
                progress.update();
                patched = new File(apk.getParent(), "patched.apk");
                try {
                    JarMap jarMap = new JarMap(apk);
                    PatchAPK.patchPackageID(jarMap, BuildConfig.APPLICATION_ID, mm.getPackageName());
                    SignAPK.sign(jarMap, new BufferedOutputStream(new FileOutputStream(patched)));
                } catch (Exception e) {
                    return;
                }
            }
            progress.dismiss();
            APKInstall.install(mm, patched);
        }
    }

    private static class RestoreManager extends ManagerDownloadListener {

        @Override
        public void onDownloadComplete(File apk, NotificationProgress progress) {
            progress.dismiss();
            Data.exportPrefs();
            if (ShellUtils.fastCmdResult("pm install " + apk))
                RootUtils.rmAndLaunch(Data.MM().getPackageName(), BuildConfig.APPLICATION_ID);
        }
    }
}
