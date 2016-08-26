package com.topjohnwu.magisk;

import android.Manifest;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
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

import com.topjohnwu.magisk.utils.Shell;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskFragment extends Fragment {

    private static final String JSON_UPDATE_CHECK = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/master/app/magisk_update.json";

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
            new DownloadFile(getContext(), mLastLink, mLastIsApp).execute();
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

        clickView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                new AlertDialog.Builder(getContext())
                        .setTitle(R.string.update_available)
                        .setMessage(Html.fromHtml(msg))
                        .setCancelable(false)
                        .setPositiveButton(R.string.update, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                mLastLink = link;
                                mLastIsApp = app;

                                if (ActivityCompat.checkSelfPermission(getContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                                        && Build.VERSION.SDK_INT >= 23) {
                                    requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
                                    return;
                                }

                                new DownloadFile(getContext(), link, app).execute();
                            }
                        })
                        .setNegativeButton(R.string.no_thanks, null)
                        .show();
            }
        });
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

            progressBar.setVisibility(View.GONE);

            version = Shell.sh("getprop magisk.version");

            if (version.isEmpty()) {
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

                String v = version.isEmpty() ? "" : version.get(0);

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

    private class DownloadFile extends AsyncTask<Void, Integer, Boolean> {

        private final Context context;
        private final String link;
        private final File downloadFile;
        private final ProgressDialog progress;

        public DownloadFile(Context context, String link, boolean apk) {
            this.link = link;
            this.context = context;

            File dir = new File(Environment.getExternalStorageDirectory() + "/Magisk");

            if (!dir.exists()) dir.mkdir();

            if (apk) {
                downloadFile = new File(dir + "/MagiskManager.apk");
            } else {
                downloadFile = new File(dir + "/Magisk.zip");
            }

            Toast.makeText(context, downloadFile.getPath(), Toast.LENGTH_SHORT).show();

            progress = new ProgressDialog(getContext());
            progress.setTitle(null);
            progress.setMessage(getString(R.string.loading));
            progress.setIndeterminate(true);
            progress.setCancelable(false);
            progress.setButton(android.app.AlertDialog.BUTTON_POSITIVE, getString(android.R.string.cancel), new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialogInterface, int i) {
                    cancel(true);
                }
            });
            progress.show();
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            try {
                URL u = new URL(link);
                URLConnection conn = u.openConnection();
                conn.connect();

                int length = conn.getContentLength();

                InputStream input = new BufferedInputStream(u.openStream(), 8192);
                OutputStream output = new FileOutputStream(downloadFile);

                byte data[] = new byte[1024];
                long total = 0;

                int count;
                while ((count = input.read(data)) != -1) {
                    total += count;
                    output.write(data, 0, count);

                    publishProgress((int) ((total * 100) / length));
                }

                output.flush();

                output.close();
                input.close();

                return true;
            } catch (IOException e) {
                return false;
            }
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);

            progress.setMessage(getString(R.string.loading) + " " + values[0] + "%");
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);
            progress.dismiss();
            if (!result) {
                Toast.makeText(context, R.string.error_download_file, Toast.LENGTH_LONG).show();
                return;
            }

            if (downloadFile.getPath().contains("apk")) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.fromFile(downloadFile), "application/vnd.android.package-archive");
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
            } else {
                Toast.makeText(context, R.string.flash_recovery, Toast.LENGTH_LONG).show();
            }

        }
    }
}
