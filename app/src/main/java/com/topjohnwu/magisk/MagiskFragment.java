package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.Intent;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;

import android.support.v4.content.FileProvider;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskFragment extends Fragment {

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

    @BindColor(R.color.grey500) int grey500;

    private int colorOK, colorWarn, colorNeutral;
    int statusOK = R.drawable.ic_check_circle;
    int statusUnknown = R.drawable.ic_help;
    private AlertDialog.Builder builder;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.magisk_fragment, container, false);
        ButterKnife.bind(this, v);
        int[] attrs0 = {R.attr.ColorOK};
        int[] attrs1 = {R.attr.ColorWarn};
        int[] attrs2 = {R.attr.ColorNeutral};
        TypedArray ta0 = getActivity().obtainStyledAttributes(attrs0);
        TypedArray ta1 = getActivity().obtainStyledAttributes(attrs1);
        TypedArray ta2 = getActivity().obtainStyledAttributes(attrs2);
        colorOK = ta0.getColor(0, Color.GRAY);
        colorWarn = ta1.getColor(0, Color.GRAY);
        colorNeutral = ta2.getColor(0, Color.GRAY);
        ta0.recycle();
        ta1.recycle();
        ta2.recycle();
        new updateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);

        return v;
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle("Magisk");
    }

    private class updateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);
            String theme = PreferenceManager.getDefaultSharedPreferences(getActivity()).getString("theme", "");
            if (theme.equals("Dark")) {
                builder = new AlertDialog.Builder(getActivity(),R.style.AlertDialog_dh);
            } else {
                builder = new AlertDialog.Builder(getActivity());
            }

            if (Utils.magiskVersion == -1) {
                magiskStatusContainer.setBackgroundColor(grey500);
                magiskStatusIcon.setImageResource(statusUnknown);

                magiskVersion.setTextColor(grey500);
                magiskVersion.setText(R.string.magisk_version_error);
            } else {
                magiskStatusContainer.setBackgroundColor(colorOK);
                magiskStatusIcon.setImageResource(statusOK);


                magiskVersion.setText(getString(R.string.magisk_version, String.valueOf(Utils.magiskVersion)));
                magiskVersion.setTextColor(colorOK);
            }

            if (Utils.remoteMagiskVersion == -1) {
                appCheckUpdatesContainer.setBackgroundColor(colorWarn);
                magiskCheckUpdatesContainer.setBackgroundColor(colorWarn);

                appCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);
                magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);

                appCheckUpdatesStatus.setText(R.string.cannot_check_updates);
                magiskCheckUpdatesStatus.setText(R.string.cannot_check_updates);
                magiskCheckUpdatesStatus.setTextColor(colorWarn);
            } else {
                if (Utils.remoteMagiskVersion > Utils.magiskVersion) {
                    magiskCheckUpdatesContainer.setBackgroundColor(colorNeutral);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                    magiskCheckUpdatesStatus.setText(getString(R.string.magisk_update_available, String.valueOf(Utils.remoteMagiskVersion)));
                    magiskCheckUpdatesStatus.setTextColor(colorNeutral);
                    magiskUpdateView.setOnClickListener(view -> builder
                            .setTitle(getString(R.string.update_title, getString(R.string.magisk)))
                            .setMessage(getString(R.string.update_msg, getString(R.string.magisk), String.valueOf(Utils.remoteMagiskVersion), Utils.magiskChangelog))
                            .setCancelable(true)
                            .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(
                                    getActivity(),
                                    new Utils.DownloadReceiver(getString(R.string.magisk)) {
                                        @Override
                                        public void task(File file) {
                                            new Async.FlashZIP(mContext, mName, file.getPath()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                        }
                                    },
                                    Utils.magiskLink, "latest_magisk.zip"))
                            .setNegativeButton(R.string.no_thanks, null)
                            .show());
                } else {
                    magiskCheckUpdatesContainer.setBackgroundColor(colorOK);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    magiskCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
                    magiskCheckUpdatesStatus.setTextColor(colorOK);
                }

                if (Utils.remoteAppVersion > BuildConfig.VERSION_CODE) {
                    appCheckUpdatesContainer.setBackgroundColor(colorNeutral);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                    appCheckUpdatesStatus.setText(getString(R.string.app_update_available, String.valueOf(Utils.remoteAppVersion)));
                    appCheckUpdatesStatus.setTextColor(colorNeutral);
                    appUpdateView.setOnClickListener(view -> builder
                            .setTitle(getString(R.string.update_title, getString(R.string.app_name)))
                            .setMessage(getString(R.string.update_msg, getString(R.string.app_name), String.valueOf(Utils.remoteAppVersion), Utils.appChangelog))
                            .setCancelable(true)
                            .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(getActivity(),
                                    new Utils.DownloadReceiver() {
                                        @Override
                                        public void task(File file) {
                                            Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                                            install.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                                            install.setData(FileProvider.getUriForFile(mContext, "com.topjohnwu.magisk.provider", file));
                                            mContext.startActivity(install);
                                        }
                                    },
                                    Utils.appLink, "latest_manager.apk"))
                            .setNegativeButton(R.string.no_thanks, null)
                            .show()
                    );
                } else {
                    appCheckUpdatesContainer.setBackgroundColor(colorOK);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    appCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.app_name)));
                    appCheckUpdatesStatus.setTextColor(colorOK);

                }
            }

            progressBar.setVisibility(View.GONE);
            appCheckUpdatesProgress.setVisibility(View.GONE);
            magiskCheckUpdatesProgress.setVisibility(View.GONE);
        }
    }
}
