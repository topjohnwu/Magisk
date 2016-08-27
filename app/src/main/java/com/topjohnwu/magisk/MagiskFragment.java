package com.topjohnwu.magisk;

import android.Manifest;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AlertDialog;
import android.text.Html;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.DownloadReceiver;
import com.topjohnwu.magisk.utils.Shell;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Method;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskFragment extends Fragment {

    private static final String JSON_UPDATE_CHECK = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updates/magisk_update.json";

    @BindView(R.id.progressBarVersion) ProgressBar progressBar;

    @BindView(R.id.magiskStatusView) View magiskStatusView;
    @BindView(R.id.magisk_status_container) View magiskStatusContainer;
    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersion;

    @BindView(R.id.app_updateView) View appUpdateView;
    @BindView(R.id.app_check_updates_container) View appCheckUpdatesContainer;
    @BindView(R.id.app_check_updates_icon) ImageView appCheckUpdatesIcon;
    @BindView(R.id.app_check_updates_status) TextView appCheckUpdatesStatus;
    @BindView(R.id.app_check_updates_progress) ProgressBar appCheckUpdatesProgress;

    @BindView(R.id.magisk_updateView) View magiskUpdateView;
    @BindView(R.id.magisk_check_updates_container) View magiskCheckUpdatesContainer;
    @BindView(R.id.magisk_check_updates_icon) ImageView magiskCheckUpdatesIcon;
    @BindView(R.id.magisk_check_updates_status) TextView magiskCheckUpdatesStatus;
    @BindView(R.id.magisk_check_updates_progress) ProgressBar magiskCheckUpdatesProgress;

    @BindColor(R.color.green500) int green500;
    @BindColor(R.color.accent) int accent;
    @BindColor(R.color.blue500) int blue500;
    @BindColor(R.color.grey500) int grey500;

    int statusOK = R.drawable.ic_check_circle;
    int statusUnknown = R.drawable.ic_help;

    private String mLastLink;
    private boolean mLastIsApp;
    private List<String> version;
    private long apkID, zipID;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.magisk_fragment, container, false);
        ButterKnife.bind(this, v);

        new updateUI().execute();
        new CheckUpdates().execute();

        return v;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            downloadFile();
        } else {
            Toast.makeText(getContext(), R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
        }
    }

    private void setupCardView(final boolean app, final String versionCode, final String link, String changelog) {
        View clickView;
        if (app) {
            clickView = appUpdateView;
            appCheckUpdatesContainer.setBackgroundColor(blue500);
            appCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
            appCheckUpdatesStatus.setText(R.string.app_update_available);
        } else {
            clickView = magiskUpdateView;
            magiskCheckUpdatesContainer.setBackgroundColor(blue500);
            magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
            magiskCheckUpdatesStatus.setText(R.string.magisk_update_available);
        }

        String text = app ? getString(R.string.app_name) : getString(R.string.magisk);
        final String msg = getString(R.string.update_available_message, text, versionCode, changelog);

        clickView.setOnClickListener(view -> new AlertDialog.Builder(getContext())
                .setTitle(R.string.update_available)
                .setMessage(Html.fromHtml(msg))
                .setCancelable(false)
                .setPositiveButton(R.string.update, (dialogInterface, i) -> {
                    mLastLink = link;
                    mLastIsApp = app;

                    if (ActivityCompat.checkSelfPermission(getContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
                        downloadFile();
                    } else {
                        requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
                    }

                })
                .setNegativeButton(R.string.no_thanks, null)
                .show());
    }

    private void downloadFile() {

        File downloadFile, dir = new File(Environment.getExternalStorageDirectory() + "/MagiskManager");
        DownloadReceiver receiver;

        if (mLastIsApp) {
            downloadFile = new File(dir + "/MagiskManager.apk");

        } else {
            downloadFile = new File(dir + "/Magisk.zip");
        }

        if (!dir.exists()) dir.mkdir();

        DownloadManager downloadManager = (DownloadManager) getContext().getSystemService(Context.DOWNLOAD_SERVICE);
        DownloadManager.Request request = new DownloadManager.Request(Uri.parse(mLastLink));
        request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
                .setDestinationUri(Uri.fromFile(downloadFile));

        if (downloadFile.exists()) downloadFile.delete();

        long downloadID = downloadManager.enqueue(request);

        if (mLastIsApp) {
            receiver = new DownloadReceiver(downloadID) {
                @Override
                public void task(File file) {
                    Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                    install.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    install.setData(FileProvider.getUriForFile(context, "com.topjohnwu.magisk.provider", file));
                    context.startActivity(install);
                }
            };
        } else {
            receiver = new DownloadReceiver(downloadID) {
                @Override
                public void task(final File file) {
                    new AlertDialog.Builder(context)
                            .setTitle("Reboot Recovery")
                            .setMessage("Do you want to flash in recovery now?")
                            .setCancelable(false)
                            .setPositiveButton("Yes, flash now", (dialogInterface, i) -> Toast.makeText(context, file.getPath(), Toast.LENGTH_LONG).show())
                            .setNegativeButton(R.string.no_thanks, null)
                            .show();
                }
            };
        }
        getActivity().registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
    }

    private class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            // Make sure static block invoked
            Shell.rootAccess();
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);

            version = Shell.sh("getprop magisk.version");

            if (version.get(0).replaceAll("\\s", "").isEmpty()) {
                magiskStatusContainer.setBackgroundColor(grey500);
                magiskStatusIcon.setImageResource(statusUnknown);

                magiskVersion.setTextColor(grey500);
                magiskVersion.setText(R.string.magisk_version_error);
            } else {
                magiskStatusContainer.setBackgroundColor(green500);
                magiskStatusIcon.setImageResource(statusOK);

                magiskVersion.setTextColor(green500);
                magiskVersion.setText(getString(R.string.magisk_version, version.get(0)));
            }

            progressBar.setVisibility(View.GONE);
            magiskStatusView.setVisibility(View.VISIBLE);
        }
    }

    private class CheckUpdates extends AsyncTask<Void, Void, String> {

        @Override
        protected String doInBackground(Void... voids) {
            try {
                HttpURLConnection c = (HttpURLConnection) new URL(JSON_UPDATE_CHECK).openConnection();
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
                return sb.toString();
            } catch (IOException e) {
                return null;
            }
        }

        @Override
        protected void onPostExecute(String result) {
            super.onPostExecute(result);

            appCheckUpdatesProgress.setVisibility(View.GONE);
            magiskCheckUpdatesProgress.setVisibility(View.GONE);

            if (result == null) {
                appCheckUpdatesContainer.setBackgroundColor(accent);
                magiskCheckUpdatesContainer.setBackgroundColor(accent);

                appCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);
                magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);

                appCheckUpdatesStatus.setText(R.string.cannot_check_updates);
                magiskCheckUpdatesStatus.setText(R.string.cannot_check_updates);
                return;
            }

            try {
                JSONObject json = new JSONObject(result);

                JSONObject app = json.getJSONObject("app");
                JSONObject magisk = json.getJSONObject("magisk");

                String appVersionCode = app.getString("versionCode");
                String appLink = app.getString("link");
                String appChangelog = app.getString("changelog");

                String magiskVersionCode = magisk.getString("versionCode");
                String magiskLink = magisk.getString("link");
                String magiskChangelog = magisk.getString("changelog");

                if (Integer.parseInt(appVersionCode) > BuildConfig.VERSION_CODE) {
                    setupCardView(true, appVersionCode, appLink, appChangelog);
                } else {
                    appCheckUpdatesContainer.setBackgroundColor(green500);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    appCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.app_name)));
                }

                String v = version.get(0).replaceAll("\\s", "");

                int versionInt = TextUtils.isEmpty(v) ? 0 : Integer.parseInt(v);

                if (Integer.parseInt(magiskVersionCode) > versionInt) {
                    setupCardView(false, magiskVersionCode, magiskLink, magiskChangelog);
                } else {
                    magiskCheckUpdatesContainer.setBackgroundColor(green500);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    magiskCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
                }

            } catch (JSONException ignored) {
            }
        }
    }
}
