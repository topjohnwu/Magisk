package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.provider.OpenableColumns;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.widget.Toast;

import com.topjohnwu.magisk.ModulesFragment;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.receivers.DownloadReceiver;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
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
            String jsonStr = WebRequest.makeWebServiceCall(Utils.UPDATE_JSON, WebRequest.GET);
            try {
                JSONObject json = new JSONObject(jsonStr);

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
            } catch (JSONException ignored) {}
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
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(
                                mContext,
                                new DownloadReceiver(mContext.getString(R.string.magisk)) {
                                    @Override
                                    public void task(Uri uri) {
                                        new FlashZIP(mContext, uri).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                    }
                                },
                                Utils.magiskLink,
                                "latest_magisk.zip"))
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
            Logger.dev("Loading modules");
            List<String> magisk = Utils.getModList(Utils.MAGISK_PATH);
            List<String> magiskCache = Utils.getModList(Utils.MAGISK_CACHE_PATH);

            for (String mod : magisk) {
                Logger.dev("Adding modules from " + mod);
                ModulesFragment.listModules.add(new Module(mod));
            }

            for (String mod : magiskCache) {
                Logger.dev("Adding cache modules from " + mod);
                Module cacheMod = new Module(mod);
                // Prevent people forgot to change module.prop
                cacheMod.setCache();
                ModulesFragment.listModules.add(cacheMod);
            }

            Collections.sort(ModulesFragment.listModules, new Utils.ModuleComparator());

            Logger.dev("Module load done");

            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
            prefs.edit().putBoolean("module_done", true).apply();
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

        @Override
        protected void onPostExecute(Void v) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
            prefs.edit().putBoolean("repo_done", true).apply();
        }
    }

    public static class FlashZIP extends AsyncTask<Void, Void, Boolean> {

        private String mName;
        private Uri mUri;
        private ProgressDialog progress;
        private File mFile, sdFile;
        private Context mContext;
        private List<String> ret;
        private boolean copyToSD;

        public FlashZIP(Context context, Uri uri, String name) {
            mContext = context;
            mUri = uri;
            mName = name;
            copyToSD = true;
        }

        public FlashZIP(Context context, Uri uri) {
            mContext = context;
            mUri = uri;
            Cursor c = mContext.getContentResolver().query(uri, null, null, null, null);
            int nameIndex = c.getColumnIndex(OpenableColumns.DISPLAY_NAME);
            c.moveToFirst();
            mName = c.getString(nameIndex);
            c.close();
            copyToSD = false;
        }

        private static void createFileFromInputStream(InputStream inputStream, File f) throws IOException {
            if (f.exists() && !f.delete()) {
                throw new IOException();
            }
            f.setWritable(true, false);
            OutputStream outputStream = new FileOutputStream(f);
            byte buffer[] = new byte[1024];
            int length;

            while ((length = inputStream.read(buffer)) > 0) {
                outputStream.write(buffer, 0, length);
            }

            outputStream.close();
            Logger.dev("FlashZip: File created successfully - " + f.getPath());
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progress = ProgressDialog.show(mContext, mContext.getString(R.string.zip_install_progress_title), mContext.getString(R.string.zip_install_progress_msg, mName));
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            Logger.dev("FlashZip Running... " + mName);
            InputStream in;
            try {
                try {
                    in = mContext.getContentResolver().openInputStream(mUri);
                    mFile = new File(mContext.getFilesDir().getAbsolutePath() + "/install.zip");
                    createFileFromInputStream(in, mFile);
                } catch (FileNotFoundException e) {
                    Log.e("Magisk", "FlashZip: Invalid Uri");
                    throw e;
                } catch (IOException e) {
                    Log.e("Magisk", "FlashZip: Error in creating file");
                    throw e;
                }
            } catch (Throwable e) {
                this.cancel(true);
                progress.cancel();
                e.printStackTrace();
                return false;
            }
            Logger.dev(mName + "; " + mFile.getPath());
//            return false;
            if (Shell.rootAccess()) {
                ret = Shell.su(
                        "unzip -o " + mFile.getPath() + " META-INF/com/google/android/* -d " + mFile.getParent(),
                        "BOOTMODE=true sh " + mFile.getParent() + "/META-INF/com/google/android/update-binary dummy 1 "+ mFile.getPath(),
                        "if [ $? -eq 0 ]; then echo true; else echo false; fi"
                );
                Logger.dev("FlashZip: Console log:");
                for (String line : ret) {
                    Logger.dev(line);
                }
            }
            // Copy the file to sdcard
            if (copyToSD) {
                try {
                    sdFile = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + (mName.contains(".zip") ? mName : mName + ".zip"));
                    if ((!sdFile.getParentFile().exists() && !sdFile.getParentFile().mkdirs()) || (sdFile.exists() && !sdFile.delete())) {
                        throw new IOException();
                    }
                    createFileFromInputStream(in, sdFile);
                    assert in != null;
                    in.close();
                } catch (IOException e) {
                    Log.e("Magisk", "FlashZip: Unable to copy to sdcard");
                    e.printStackTrace();
                }
            }
            mFile.delete();
            return ret != null && Boolean.parseBoolean(ret.get(ret.size() - 1));
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);
            progress.dismiss();
            if (!result) {
                if (sdFile == null) {
                    Toast.makeText(mContext, mContext.getString(R.string.install_error), Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(mContext, mContext.getString(R.string.manual_install, mFile.getAbsolutePath()), Toast.LENGTH_LONG).show();
                }
            } else {
                done();
            }
        }

        protected void done() {
            AlertDialog.Builder builder;
            String theme = PreferenceManager.getDefaultSharedPreferences(mContext).getString("theme", "");
            if (theme.equals("Dark")) {
                builder = new AlertDialog.Builder(mContext, R.style.AlertDialog_dh);
            } else {
                builder = new AlertDialog.Builder(mContext);
            }
            builder
                    .setTitle(R.string.reboot_title)
                    .setMessage(R.string.reboot_msg)
                    .setPositiveButton(R.string.reboot, (dialogInterface1, i) -> Shell.su("sh -c reboot"))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }
}
