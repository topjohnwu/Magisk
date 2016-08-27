package com.topjohnwu.magisk;

import android.Manifest;
import android.app.DownloadManager;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AlertDialog;
import android.text.Html;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.topjohnwu.magisk.receivers.ApkReceiver;
import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.receivers.ZipReceiver;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
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

    private String mLastLink, mLastFile;
    private boolean mLastIsApp;
    private List<String> version;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.magisk_fragment, container, false);
        ButterKnife.bind(this, v);

        new updateUI().execute();
        new CheckUpdates().execute();

        return v;
    }

    private void setListener(View clickView, DownloadReceiver receiver, String link, String msg, String file) {
        clickView.setOnClickListener(view -> new AlertDialog.Builder(getContext())
                .setTitle(R.string.update_available)
                .setMessage(Html.fromHtml(msg))
                .setCancelable(false)
                .setPositiveButton(R.string.download, (dialogInterface, i) -> {

                    if (ActivityCompat.checkSelfPermission(getContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                        Toast.makeText(getContext(), R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
                        return;
                    }

                    File downloadFile, dir = new File(Environment.getExternalStorageDirectory() + "/MagiskManager");

                    downloadFile = new File(dir + "/" + file);

                    if (!dir.exists()) dir.mkdir();

                    DownloadManager downloadManager = (DownloadManager) getContext().getSystemService(Context.DOWNLOAD_SERVICE);
                    DownloadManager.Request request = new DownloadManager.Request(Uri.parse(link));
                    request.setDestinationUri(Uri.fromFile(downloadFile));

                    if (downloadFile.exists()) downloadFile.delete();

                    receiver.setDownloadID(downloadManager.enqueue(request));

                    getActivity().registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
                })
                .setNegativeButton(R.string.no_thanks, null)
                .show());
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

            if (Shell.magiskVersion == -1) {
                magiskStatusContainer.setBackgroundColor(grey500);
                magiskStatusIcon.setImageResource(statusUnknown);

                magiskVersion.setTextColor(grey500);
                magiskVersion.setText(R.string.magisk_version_error);
            } else {
                magiskStatusContainer.setBackgroundColor(green500);
                magiskStatusIcon.setImageResource(statusOK);

                magiskVersion.setTextColor(green500);
                magiskVersion.setText(getString(R.string.magisk_version, String.valueOf(Shell.magiskVersion)));
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

            magiskUpdateView.setVisibility(View.VISIBLE);
            appUpdateView.setVisibility(View.VISIBLE);

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

                //if (Integer.parseInt(magiskVersionCode) > versionInt) {
                if (99 > Shell.magiskVersion) {
                    magiskCheckUpdatesContainer.setBackgroundColor(blue500);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                    magiskCheckUpdatesStatus.setText(getString(R.string.magisk_update_available, magiskVersionCode));
                    setListener(magiskUpdateView, new ZipReceiver(), "https://www.dropbox.com/s/dc16jf1ifhv6ef4/Magisk-v7.zip?dl=1",
                            getString(R.string.update_available_message, "Magisk", appVersionCode, magiskChangelog),
                            "latest_magisk.zip"
                    );
                } else {
                    magiskCheckUpdatesContainer.setBackgroundColor(green500);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    magiskCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
                }

                if (Integer.parseInt(appVersionCode) > BuildConfig.VERSION_CODE) {
                    appCheckUpdatesContainer.setBackgroundColor(blue500);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                    appCheckUpdatesStatus.setText(getString(R.string.app_update_available, appVersionCode));
                    setListener(appUpdateView, new ApkReceiver(), appLink,
                            getString(R.string.update_available_message, "Magisk Manager", appVersionCode, appChangelog),
                            "latest_manager.apk"
                    );

                } else {
                    appCheckUpdatesContainer.setBackgroundColor(green500);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    appCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.app_name)));
                }


            } catch (JSONException ignored) {
            }
        }
    }
}
