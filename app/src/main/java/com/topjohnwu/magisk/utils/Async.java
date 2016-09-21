package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.DocumentsContract;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.widget.Toast;

import com.topjohnwu.magisk.ModulesFragment;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.RepoHelper;

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
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> {
                            new AlertDialog.Builder(mContext)
                                    .setTitle(R.string.root_method_title)
                                    .setItems(new String[]{mContext.getString(R.string.phh), mContext.getString(R.string.supersu)}, (dialogInterface1, root) -> {
                                        Utils.DownloadReceiver rootReceiver;
                                        String link, filename;
                                        switch (root) {
                                            case 0:
                                                link = Utils.phhLink;
                                                filename = "phhsu.zip";
                                                rootReceiver = new Utils.DownloadReceiver(mContext.getString(R.string.phh)) {
                                                    @Override
                                                    public void task(File file) {
                                                        new FlashZIP(mContext, mName, file.getPath()).execute();
                                                    }
                                                };
                                                break;
                                            case 1:
                                                link = Utils.supersuLink;
                                                filename = "supersu.zip";
                                                rootReceiver = new Utils.DownloadReceiver(mContext.getString(R.string.supersu)) {
                                                    @Override
                                                    public void task(File file) {
                                                        new FlashZIP(mContext, mName, file.getPath()).execute();
                                                    }
                                                };
                                                break;
                                            default:
                                                rootReceiver = null;
                                                link = filename = null;
                                        }
                                        Utils.DownloadReceiver magiskReceiver = new Utils.DownloadReceiver(mContext.getString(R.string.magisk)) {
                                            @Override
                                            public void task(File file) {
                                                Context temp = mContext;
                                                new FlashZIP(mContext, mName, file.getPath()) {
                                                    @Override
                                                    protected void done() {
                                                        Utils.downloadAndReceive(temp, rootReceiver, link, filename);
                                                    }
                                                }.execute();
                                            }
                                        };
                                        Utils.downloadAndReceive(mContext, magiskReceiver, Utils.magiskLink, "latest_magisk.zip");
                                    })
                                    .show();
                        })
                        .setNegativeButton(R.string.no_thanks, null)
                        .show();
            } else if (Shell.rootStatus == 2) {
                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.root_system)
                        .setMessage(R.string.root_system_msg)
                        .setCancelable(true)
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> {
                            new AlertDialog.Builder(mContext)
                                    .setTitle(R.string.root_method_title)
                                    .setItems(new String[]{mContext.getString(R.string.phh), mContext.getString(R.string.supersu)}, (dialogInterface1, root) -> {
                                        switch (root) {
                                            case 0:
                                                Utils.downloadAndReceive(
                                                        mContext,
                                                        new Utils.DownloadReceiver(mContext.getString(R.string.phh)) {
                                                            @Override
                                                            public void task(File file) {
                                                                new FlashZIP(mContext, mName, file.getPath()).execute();
                                                            }
                                                        },
                                                        Utils.phhLink, "phhsu.zip");
                                                break;
                                            case 1:
                                                Utils.downloadAndReceive(
                                                        mContext,
                                                        new Utils.DownloadReceiver(mContext.getString(R.string.supersu)) {
                                                            @Override
                                                            public void task(File file) {
                                                                new FlashZIP(mContext, mName, file.getPath()).execute();
                                                            }
                                                        },
                                                        Utils.supersuLink, "supersu.zip");
                                                break;
                                        }
                                    })
                                    .show();
                        })
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
            List<String> magisk = Utils.getModList(Utils.MAGISK_PATH);
            Log.d("Magisk", "Utils: Reload called, loading modules");
            List<String> magiskCache = Utils.getModList(Utils.MAGISK_CACHE_PATH);

            for (String mod : magisk) {
                Log.d("Magisk", "Utils: Adding module from string " + mod);
                ModulesFragment.listModules.add(new Module(mContext, mod));
            }

            for (String mod : magiskCache) {
                Log.d("Magisk", "Utils: Adding cache module from string " + mod);
                Module cacheMod = new Module(mContext, mod);
                // Prevent people forgot to change module.prop
                cacheMod.setCache();
                ModulesFragment.listModules.add(cacheMod);
            }

            Collections.sort(ModulesFragment.listModules, new Utils.ModuleComparator());

            return null;
        }

    }

    public static class LoadRepos extends AsyncTask<Void, Void, Void> {

        private Context mContext;
        private boolean doReload;
        private RepoHelper.TaskDelegate mTaskDelegate;

        public LoadRepos(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ReposFragment.mListRepos.clear();
            RepoHelper.createRepoMap(mContext);
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
            String file = "";
            final String docId = DocumentsContract.getDocumentId(mUri);

            Log.d("Magisk", "Utils: FlashZip Running, " + docId + " and " + mUri.toString());
            if (docId.contains(":"))
                mName = docId.split(":")[1];
            else mName = docId;
            if (mName.contains("/"))
                mName = mName.substring(mName.lastIndexOf('/') + 1);
            if (mName.contains(".zip")) {
                file = mContext.getFilesDir() + "/" + mName;
                Log.d("Magisk", "Utils: FlashZip running for uRI " + mUri.toString());
            } else {
                Log.e("Magisk", "Utils: error parsing Zipfile " + mUri.getPath());
                this.cancel(true);
            }
            ContentResolver contentResolver = mContext.getContentResolver();
            //contentResolver.takePersistableUriPermission(mUri, flags);
            try {
                InputStream in = contentResolver.openInputStream(mUri);
                Log.d("Magisk", "Firing inputStream");
                mFile = createFileFromInputStream(in, file, mContext);
                if (mFile != null) {
                    mPath = mFile.getPath();
                    Log.d("Magisk", "Utils: Mpath is " + mPath);
                } else {
                    Log.e("Magisk", "Utils: error creating file " + mUri.getPath());
                    this.cancel(true);
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            // TODO handle non-primary volumes

        }

        private static File createFileFromInputStream(InputStream inputStream, String fileName, Context context) {

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
                Log.d("Magisk", "Holy balls, I think it worked.  File is " + f.getPath());
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
            if (mPath != null) {
                Log.e("Magisk", "Utils: Error, flashZIP called without a valid zip file to flash.");
                progress.dismiss();
                this.cancel(true);
                return false;
            }
            if (!Shell.rootAccess()) {
                return false;
            } else {
                ret = Shell.su(
                        "rm -rf /dev/tmp",
                        "mkdir -p /dev/tmp",
                        "cp -af " + mPath + " /dev/tmp/install.zip",
                        "unzip -o /dev/tmp/install.zip META-INF/com/google/android/* -d /dev/tmp",
                        "BOOTMODE=true sh /dev/tmp/META-INF/com/google/android/update-binary dummy 1 /dev/tmp/install.zip",
                        "if [ $? -eq 0 ]; then echo true; else echo false; fi"
                );
                return ret != null && Boolean.parseBoolean(ret.get(ret.size() - 1));
            }
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);
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
            new AlertDialog.Builder(mContext)
                    .setTitle(R.string.reboot_title)
                    .setMessage(R.string.reboot_msg)
                    .setPositiveButton(R.string.reboot, (dialogInterface1, i) -> Shell.su("reboot"))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }
}
