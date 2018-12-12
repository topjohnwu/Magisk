package com.topjohnwu.magisk.utils;

import android.os.AsyncTask;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.PatchAPK;
import com.topjohnwu.magisk.components.ProgressNotification;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.ResponseListener;
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

    private static void dlInstall(String name, ManagerDownloadListener listener) {
        MagiskManager mm = Data.MM();
        File apk = new File(mm.getFilesDir(), "manager.apk");
        ProgressNotification progress = new ProgressNotification(name);
        listener.setProgressNotification(progress);
        Networking.get(Data.managerLink)
                .setExecutor(AsyncTask.THREAD_POOL_EXECUTOR)
                .setDownloadProgressListener(progress)
                .setErrorHandler((conn, e) -> progress.dlFail())
                .getAsFile(listener, apk);
    }

    abstract static class ManagerDownloadListener implements ResponseListener<File> {

        private ProgressNotification progress;

        private void setProgressNotification(ProgressNotification progress) {
            this.progress = progress;
        }

        public abstract void onDownloadComplete(File apk, ProgressNotification progress);

        @Override
        public void onResponse(File apk) {
            onDownloadComplete(apk, progress);
        }
    }

    static class PatchPackageName extends ManagerDownloadListener {

        @Override
        public void onDownloadComplete(File apk, ProgressNotification progress) {
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
                    PatchAPK.patch(jarMap, mm.getPackageName());
                    SignAPK.sign(jarMap, new BufferedOutputStream(new FileOutputStream(patched)));
                } catch (Exception e) {
                    return;
                }
            }
            APKInstall.install(mm, patched);
            progress.dismiss();
        }
    }

    static class RestoreManager extends ManagerDownloadListener {

        @Override
        public void onDownloadComplete(File apk, ProgressNotification progress) {
            MagiskManager mm = Data.MM();
            progress.getNotification()
                    .setProgress(0, 0, true)
                    .setContentTitle(mm.getString(R.string.restore_img_msg))
                    .setContentText("");
            progress.update();
            Data.exportPrefs();
            // Make it world readable
            apk.setReadable(true, false);
            if (ShellUtils.fastCmdResult("pm install " + apk))
                RootUtils.rmAndLaunch(mm.getPackageName(), BuildConfig.APPLICATION_ID);
            progress.dismiss();
        }
    }
}
