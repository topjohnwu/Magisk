package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
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

    @BindColor(R.color.green500) int green500;
    @BindColor(R.color.accent) int accent;
    @BindColor(R.color.blue500) int blue500;
    @BindColor(R.color.grey500) int grey500;

    int statusOK = R.drawable.ic_check_circle;
    int statusUnknown = R.drawable.ic_help;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.magisk_fragment, container, false);
        ButterKnife.bind(this, v);

        new updateUI().execute();

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

            if (Utils.magiskVersion == -1) {
                magiskStatusContainer.setBackgroundColor(grey500);
                magiskStatusIcon.setImageResource(statusUnknown);

                magiskVersion.setTextColor(grey500);
                magiskVersion.setText(R.string.magisk_version_error);
            } else {
                magiskStatusContainer.setBackgroundColor(green500);
                magiskStatusIcon.setImageResource(statusOK);

                magiskVersion.setTextColor(green500);
                magiskVersion.setText(getString(R.string.magisk_version, String.valueOf(Utils.magiskVersion)));
            }

            if (Utils.remoteMagiskVersion == -1) {
                appCheckUpdatesContainer.setBackgroundColor(accent);
                magiskCheckUpdatesContainer.setBackgroundColor(accent);

                appCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);
                magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);

                appCheckUpdatesStatus.setText(R.string.cannot_check_updates);
                magiskCheckUpdatesStatus.setText(R.string.cannot_check_updates);
            } else {
                if (Utils.remoteMagiskVersion > Utils.magiskVersion) {
                    magiskCheckUpdatesContainer.setBackgroundColor(blue500);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                    magiskCheckUpdatesStatus.setText(getString(R.string.magisk_update_available, String.valueOf(Utils.remoteMagiskVersion)));
                    magiskUpdateView.setOnClickListener(view -> new AlertDialog.Builder(getActivity())
                            .setTitle(getString(R.string.update_title, getString(R.string.magisk)))
                            .setMessage(getString(R.string.update_msg, getString(R.string.magisk), String.valueOf(Utils.remoteMagiskVersion), Utils.magiskChangelog))
                            .setCancelable(true)
                            .setPositiveButton(R.string.download_install, (dialogInterface, i) -> {
                                Utils.downloadAndReceive(
                                        getActivity(),
                                        new Utils.DownloadReceiver(getString(R.string.magisk)) {
                                            @Override
                                            public void task(File file) {
                                                new Async.FlashZIP(mContext, mName, file.getPath()).execute();
                                            }
                                        },
                                        Utils.magiskLink, "latest_magisk.zip");
                            })
                            .setNegativeButton(R.string.no_thanks, null)
                            .show());
                } else {
                    magiskCheckUpdatesContainer.setBackgroundColor(green500);
                    magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    magiskCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
                }

                if (Utils.remoteAppVersion > BuildConfig.VERSION_CODE) {
                    appCheckUpdatesContainer.setBackgroundColor(blue500);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                    appCheckUpdatesStatus.setText(getString(R.string.app_update_available, String.valueOf(Utils.remoteAppVersion)));
                    appUpdateView.setOnClickListener(view -> new AlertDialog.Builder(getActivity())
                            .setTitle(getString(R.string.update_title, getString(R.string.app_name)))
                            .setMessage(getString(R.string.update_msg, getString(R.string.app_name), String.valueOf(Utils.remoteAppVersion), Utils.appChangelog))
                            .setCancelable(true)
                            .setPositiveButton(R.string.download_install, (dialogInterface, i) -> {
                                Utils.downloadAndReceive(getActivity(),
                                        new Utils.DownloadReceiver() {
                                            @Override
                                            public void task(File file) {
                                                Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                                                install.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                                                install.setData(FileProvider.getUriForFile(mContext, "com.topjohnwu.magisk.provider", file));
                                                mContext.startActivity(install);
                                            }
                                        },
                                        Utils.appLink, "latest_manager.apk");
                            })
                            .setNegativeButton(R.string.no_thanks, null)
                            .show()
                    );
                } else {
                    appCheckUpdatesContainer.setBackgroundColor(green500);
                    appCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                    appCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.app_name)));
                }
            }

            progressBar.setVisibility(View.GONE);
            appCheckUpdatesProgress.setVisibility(View.GONE);
            magiskCheckUpdatesProgress.setVisibility(View.GONE);
        }
    }
}
