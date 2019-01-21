package com.topjohnwu.magisk.utils;

import android.os.AsyncTask;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Data;
import com.topjohnwu.core.utils.RootUtils;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.ProgressNotification;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.ResponseListener;
import com.topjohnwu.superuser.ShellUtils;

import java.io.File;

import dalvik.system.DexClassLoader;

public class DownloadApp {

    public static void upgrade(String name) {
        dlInstall(name, new PatchPackageName());
    }

    public static void restore() {
        String name = Utils.fmt("MagiskManager v%s(%d)",
                Data.remoteManagerVersionString, Data.remoteManagerVersionCode);
        dlInstall(name, new RestoreManager());
    }

    private static void dlInstall(String name, ManagerDownloadListener listener) {
        File apk = new File(App.self.getCacheDir(), "manager.apk");
        ProgressNotification progress = new ProgressNotification(name);
        listener.progress = progress;
        Networking.get(Data.managerLink)
                .setExecutor(AsyncTask.THREAD_POOL_EXECUTOR)
                .setDownloadProgressListener(progress)
                .setErrorHandler((conn, e) -> progress.dlFail())
                .getAsFile(apk, listener);
    }

    private abstract static class ManagerDownloadListener implements ResponseListener<File> {
        ProgressNotification progress;
    }

    private static class PatchPackageName extends ManagerDownloadListener {

        @Override
        public void onResponse(File apk) {
            File patched = apk;
            App app = App.self;
            if (!app.getPackageName().equals(BuildConfig.APPLICATION_ID)) {
                progress.getNotificationBuilder()
                        .setProgress(0, 0, true)
                        .setContentTitle(app.getString(R.string.hide_manager_title))
                        .setContentText("");
                progress.update();
                patched = new File(apk.getParent(), "patched.apk");
                try {
                    // Try using the new APK to patch itself
                    ClassLoader loader = new DexClassLoader(apk.getPath(),
                            apk.getParent(), null, ClassLoader.getSystemClassLoader());
                    loader.loadClass("a.a")
                            .getMethod("patchAPK", String.class, String.class, String.class)
                            .invoke(null, apk.getPath(), patched.getPath(), app.getPackageName());
                } catch (Exception e) {
                    e.printStackTrace();
                    // Fallback to use the current implementation
                    PatchAPK.patch(apk.getPath(), patched.getPath(), app.getPackageName());
                }
            }
            APKInstall.install(app, patched);
            progress.dismiss();
        }
    }

    private static class RestoreManager extends ManagerDownloadListener {

        @Override
        public void onResponse(File apk) {
            App app = App.self;
            progress.getNotificationBuilder()
                    .setProgress(0, 0, true)
                    .setContentTitle(app.getString(R.string.restore_img_msg))
                    .setContentText("");
            progress.update();
            Data.exportPrefs();
            // Make it world readable
            apk.setReadable(true, false);
            if (ShellUtils.fastCmdResult("pm install " + apk))
                RootUtils.rmAndLaunch(app.getPackageName(), BuildConfig.APPLICATION_ID);
            progress.dismiss();
        }
    }
}
