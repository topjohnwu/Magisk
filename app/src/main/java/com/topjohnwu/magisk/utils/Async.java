package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.widget.Toast;

import com.ipaulpro.afilechooser.FileInfo;
import com.ipaulpro.afilechooser.utils.FileUtils;
import com.topjohnwu.magisk.ModulesFragment;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.RepoHelper;
import com.topjohnwu.magisk.receivers.DownloadReceiver;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Collections;
import java.util.List;

public class Async {

    public static class BusyboxEnv extends AsyncTask<Void, Void, Void> {

        Context mContext;

        public BusyboxEnv(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String toolPath = mContext.getApplicationInfo().dataDir + "/busybox";
            String busybox = mContext.getApplicationInfo().dataDir + "/lib/libbusybox.so";
            if (Shell.rootAccess() && !Utils.commandExists("unzip") && !Utils.itemExist(toolPath)) {
                Shell.su(true,
                        "mkdir " + toolPath,
                        "chmod 755 " + toolPath,
                        "ln -s " + busybox + " " + toolPath + "/busybox",
                        "for tool in $(" + toolPath + "/busybox --list); do",
                        "ln -s " + busybox + " " + toolPath + "/$tool",
                        "done"
                );
            }
            return null;
        }
    }

    public static class CheckUpdates extends AsyncTask<Void, Void, Void> {

        private Context mContext;

        public CheckUpdates(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            try {
                HttpURLConnection c = (HttpURLConnection) new URL(Utils.UPDATE_JSON).openConnection();
                c.setRequestMethod("GET");
                c.setInstanceFollowRedirects(false);
                c.setDoOutput(false);
                c.connect();

                BufferedReader br = new BufferedReader(new InputStreamReader(c.getInputStream()));
                StringBuilder sb = new StringBuilder();
                String line;
                while ((line = br.readLine()) != null) {
                    sb.append(line);
                }
                br.close();
                JSONObject json = new JSONObject(sb.toString());
                JSONObject magisk = json.getJSONObject("magisk");
                JSONObject app = json.getJSONObject("app");
                JSONObject root = json.getJSONObject("root");

                Utils.remoteMagiskVersion = magisk.getInt("versionCode");
                Utils.magiskLink = magisk.getString("link");
                Utils.magiskChangelog = magisk.getString("changelog");

                Utils.remoteAppVersion = app.getInt("versionCode");
                Utils.appLink = app.getString("link");
                Utils.appChangelog = app.getString("changelog");

                Utils.phhLink = root.getString("phh");
                Utils.supersuLink = root.getString("supersu");

            } catch (IOException | JSONException ignored) {
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            if (Shell.rootAccess() && Utils.magiskVersion == -1) {
                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.no_magisk_title)
                        .setMessage(R.string.no_magisk_msg)
                        .setCancelable(true)
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> new AlertDialog.Builder(mContext)
                                .setTitle(R.string.root_method_title)
                                .setItems(new String[]{mContext.getString(R.string.phh), mContext.getString(R.string.supersu)}, (dialogInterface1, root) -> {
                                    DownloadReceiver rootReceiver;
                                    String link, filename;
                                    switch (root) {
                                        case 0:
                                            link = Utils.phhLink;
                                            filename = "phhsu.zip";
                                            rootReceiver = new DownloadReceiver(mContext.getString(R.string.phh)) {
                                                @Override
                                                public void task(File file) {
                                                    new FlashZIP(mContext, mName, file.getPath()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                                }
                                            };
                                            break;
                                        case 1:
                                            link = Utils.supersuLink;
                                            filename = "supersu.zip";
                                            rootReceiver = new DownloadReceiver(mContext.getString(R.string.supersu)) {
                                                @Override
                                                public void task(File file) {
                                                    new FlashZIP(mContext, mName, file.getPath()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                                }
                                            };
                                            break;
                                        default:
                                            rootReceiver = null;
                                            link = filename = null;
                                    }
                                    DownloadReceiver magiskReceiver = new DownloadReceiver(mContext.getString(R.string.magisk)) {
                                        @Override
                                        public void task(File file) {
                                            Context temp = mContext;
                                            new FlashZIP(mContext, mName, file.getPath()) {
                                                @Override
                                                protected void done() {
                                                    Utils.downloadAndReceive(temp, rootReceiver, link, filename);
                                                }
                                            }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                        }
                                    };
                                    Utils.downloadAndReceive(mContext, magiskReceiver, Utils.magiskLink, "latest_magisk.zip");
                                })
                                .show())
                        .setNegativeButton(R.string.no_thanks, null)
                        .show();
            } else if (Shell.rootStatus == 2) {
                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.root_system)
                        .setMessage(R.string.root_system_msg)
                        .setCancelable(true)
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> new AlertDialog.Builder(mContext)
                                .setTitle(R.string.root_method_title)
                                .setItems(new String[]{mContext.getString(R.string.phh), mContext.getString(R.string.supersu)}, (dialogInterface1, root) -> {
                                    switch (root) {
                                        case 0:
                                            Utils.downloadAndReceive(
                                                    mContext,
                                                    new DownloadReceiver(mContext.getString(R.string.phh)) {
                                                        @Override
                                                        public void task(File file) {
                                                            new FlashZIP(mContext, mName, file.getPath()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                                        }
                                                    },
                                                    Utils.phhLink, "phhsu.zip");
                                            break;
                                        case 1:
                                            Utils.downloadAndReceive(
                                                    mContext,
                                                    new DownloadReceiver(mContext.getString(R.string.supersu)) {
                                                        @Override
                                                        public void task(File file) {
                                                            new FlashZIP(mContext, mName, file.getPath()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                                        }
                                                    },
                                                    Utils.supersuLink, "supersu.zip");
                                            break;
                                    }
                                })
                                .show())
                        .setNegativeButton(R.string.no_thanks, null)
                        .show();
            }
        }
    }

    public static class LoadModules extends AsyncTask<Void, Void, Void> {

        private Context mContext;

        public LoadModules(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ModulesFragment.listModules.clear();
            Logger.dh("Loading modules");
            List<String> magisk = Utils.getModList(Utils.MAGISK_PATH);
            List<String> magiskCache = Utils.getModList(Utils.MAGISK_CACHE_PATH);

            for (String mod : magisk) {
                Logger.dh("Adding modules from " + mod);
                ModulesFragment.listModules.add(new Module(mContext, mod));
            }

            for (String mod : magiskCache) {
                Logger.dh("Adding cache modules from " + mod);
                Module cacheMod = new Module(mContext, mod);
                // Prevent people forgot to change module.prop
                cacheMod.setCache();
                ModulesFragment.listModules.add(cacheMod);
            }

            Collections.sort(ModulesFragment.listModules, new Utils.ModuleComparator());

            Logger.dh("Module load done");

            return null;
        }

    }

    public static class LoadRepos extends AsyncTask<Void, Void, Void> {

        private Context mContext;

        public LoadRepos(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ReposFragment.mListRepos.clear();
            RepoHelper.createRepoMap(mContext);
            RepoHelper.checkUpdate();
            ReposFragment.mListRepos = RepoHelper.getSortedList();
            return null;
        }

    }

    public static class FlashZIP extends AsyncTask<Void, Void, Boolean> {

        private String mPath, mName;
        private Uri mUri;
        private ProgressDialog progress;
        private File mFile;
        private Context mContext;
        private List<String> ret;
        private boolean deleteFileAfter;

        public FlashZIP(Context context, String name, String path) {
            mContext = context;
            mName = name;
            mPath = path;
            deleteFileAfter = false;
        }

        public FlashZIP(Context context, Uri uRi) {
            mContext = context;
            mUri = uRi;
            deleteFileAfter = true;

            String file;
            FileInfo fileInfo = FileUtils.getFileInfo(context, mUri);

            Logger.dh("Utils: FlashZip Running, " + fileInfo.getPath());
            String filename = fileInfo.getPath();
            String idStr = filename.substring(filename.lastIndexOf('/') + 1);
            if (!idStr.contains(".zip")) {
                Logger.dh("Async: Improper name, cancelling " + idStr);
                this.cancel(true);
                progress.cancel();
            }
            file = mContext.getFilesDir() + "/" + idStr;

            ContentResolver contentResolver = mContext.getContentResolver();
            //contentResolver.takePersistableUriPermission(mUri, flags);
            try {
                InputStream in = contentResolver.openInputStream(mUri);
                Log.d("Magisk", "Firing inputStream");
                mFile = createFileFromInputStream(in, file);
                if (mFile != null) {
                    mPath = mFile.getPath();
                    Logger.dh("Async: Mpath is " + mPath);
                } else {
                    Log.e("Magisk", "Async: error creating file " + mUri.getPath());
                    this.cancel(true);
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            // TODO handle non-primary volumes

        }

        private static File createFileFromInputStream(InputStream inputStream, String fileName) {

            try {
                File f = new File(fileName);
                f.setWritable(true, false);
                OutputStream outputStream = new FileOutputStream(f);
                byte buffer[] = new byte[1024];
                int length;

                while ((length = inputStream.read(buffer)) > 0) {
                    outputStream.write(buffer, 0, length);
                }

                outputStream.close();
                inputStream.close();
                Logger.dh("Async: File created successfully - " + f.getPath());
                return f;

            } catch (IOException e) {
                System.out.println("error in creating a file");
                e.printStackTrace();

            }

            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            progress = ProgressDialog.show(mContext, mContext.getString(R.string.zip_install_progress_title), mContext.getString(R.string.zip_install_progress_msg, mName));
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            if (mPath == null) {
                Log.e("Magisk", "Utils: Error, flashZIP called without a valid zip file to flash.");
                this.cancel(true);
                progress.cancel();
                return false;
            }
            if (!Shell.rootAccess()) {
                return false;
            } else {
                ret = Shell.su(
                        "rm -rf /data/tmp",
                        "mkdir -p /data/tmp",
                        "cp -af " + mPath + " /data/tmp/install.zip",
                        "unzip -o /data/tmp/install.zip META-INF/com/google/android/* -d /data/tmp",
                        "BOOTMODE=true sh /data/tmp/META-INF/com/google/android/update-binary dummy 1 /data/tmp/install.zip",
                        "if [ $? -eq 0 ]; then echo true; else echo false; fi"
                );
                Logger.dh("Async: ret is " + ret);
                return ret != null && Boolean.parseBoolean(ret.get(ret.size() - 1));
            }
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);
            //Shell.su("rm -rf /data/tmp");
            if (deleteFileAfter) {
                Shell.su("rm -rf " + mPath);
                Log.d("Magisk", "Utils: Deleting file " + mPath);
            }
            progress.dismiss();
            if (!result) {
                Toast.makeText(mContext, mContext.getString(R.string.manual_install, mPath), Toast.LENGTH_LONG).show();
                return;
            }
            done();
        }

        protected void done() {
            AlertDialog.Builder builder;
            String theme = PreferenceManager.getDefaultSharedPreferences(mContext).getString("theme", "");
            if (theme.equals("Dark")) {
                builder = new AlertDialog.Builder(mContext,R.style.AlertDialog_dh);
            } else {
                builder = new AlertDialog.Builder(mContext);
            }
            builder
                    .setTitle(R.string.reboot_title)
                    .setMessage(R.string.reboot_msg)
                    .setPositiveButton(R.string.reboot, (dialogInterface1, i) -> Shell.su("reboot"))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }
}
