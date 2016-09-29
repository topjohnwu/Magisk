package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;

import android.support.v4.content.FileProvider;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.topjohnwu.magisk.receivers.DownloadReceiver;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskFragment extends Fragment {

    public static int magiskVersion, remoteMagiskVersion = -1, remoteAppVersionCode = -1;
    public static String magiskLink, magiskChangelog, appLink, appChangelog, remoteAppVersion;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.magiskStatusView) View magiskStatusView;
    @BindView(R.id.magisk_status_container) View magiskStatusContainer;
    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersionText;

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
    @BindColor(android.R.color.transparent) int trans;

    private int colorOK, colorWarn, colorNeutral;
    int statusOK = R.drawable.ic_check_circle;
    int statusUnknown = R.drawable.ic_help;
    private AlertDialog.Builder builder;

    private SharedPreferences prefs;
    private SharedPreferences.OnSharedPreferenceChangeListener listener;

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

        if (magiskVersion == -1) {
            magiskStatusContainer.setBackgroundColor(grey500);
            magiskStatusIcon.setImageResource(statusUnknown);

            magiskVersionText.setTextColor(grey500);
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
            magiskStatusContainer.setBackgroundColor(colorOK);
            magiskStatusIcon.setImageResource(statusOK);

            magiskVersionText.setText(getString(R.string.magisk_version, String.valueOf(magiskVersion)));
            magiskVersionText.setTextColor(colorOK);
        }

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            prefs.edit().putBoolean("update_check_done", false).apply();

            appCheckUpdatesContainer.setBackgroundColor(trans);
            magiskCheckUpdatesContainer.setBackgroundColor(trans);

            appCheckUpdatesIcon.setImageResource(0);
            magiskCheckUpdatesIcon.setImageResource(0);

            appCheckUpdatesStatus.setText(null);
            magiskCheckUpdatesStatus.setText(null);

            appCheckUpdatesProgress.setVisibility(View.VISIBLE);
            magiskCheckUpdatesProgress.setVisibility(View.VISIBLE);

            new Async.CheckUpdates(getActivity()).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        });

        if (prefs.getBoolean("update_check_done", false)) {
            updateUI();
        }

        listener = (pref, s) -> {
            if (s.equals("update_check_done")) {
                if (pref.getBoolean(s, false)) {
                    Logger.dev("MagiskFragment: UI refresh triggered");
                    updateUI();
                }
            }
        };

        return v;
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle(R.string.magisk);
        prefs.registerOnSharedPreferenceChangeListener(listener);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }


    private void updateUI() {
        String theme = PreferenceManager.getDefaultSharedPreferences(getActivity()).getString("theme", "");
        if (theme.equals("Dark")) {
            builder = new AlertDialog.Builder(getActivity(), R.style.AlertDialog_dh);
        } else {
            builder = new AlertDialog.Builder(getActivity());
        }

        if (remoteMagiskVersion == -1) {
            appCheckUpdatesContainer.setBackgroundColor(colorWarn);
            magiskCheckUpdatesContainer.setBackgroundColor(colorWarn);

            appCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);
            magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_warning);

            appCheckUpdatesStatus.setText(R.string.cannot_check_updates);
            appCheckUpdatesStatus.setTextColor(colorWarn);
            magiskCheckUpdatesStatus.setText(R.string.cannot_check_updates);
            magiskCheckUpdatesStatus.setTextColor(colorWarn);
        } else {
            if (remoteMagiskVersion > magiskVersion) {
                magiskCheckUpdatesContainer.setBackgroundColor(colorNeutral);
                magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                magiskCheckUpdatesStatus.setText(getString(R.string.magisk_update_available, String.valueOf(remoteMagiskVersion)));
                magiskCheckUpdatesStatus.setTextColor(colorNeutral);
                magiskUpdateView.setOnClickListener(view -> builder
                        .setTitle(getString(R.string.update_title, getString(R.string.magisk)))
                        .setMessage(getString(R.string.update_msg, getString(R.string.magisk), String.valueOf(remoteMagiskVersion), magiskChangelog))
                        .setCancelable(true)
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(
                                getActivity(),
                                new DownloadReceiver() {
                                    @Override
                                    public void task(Uri uri) {
                                        new Async.FlashZIP(mContext, uri).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                    }
                                },
                                magiskLink,
                                "Magisk-v" + String.valueOf(remoteMagiskVersion) + ".zip"))
                        .setNegativeButton(R.string.no_thanks, null)
                        .show());

            } else {
                magiskCheckUpdatesContainer.setBackgroundColor(colorOK);
                magiskCheckUpdatesIcon.setImageResource(R.drawable.ic_check_circle);
                magiskCheckUpdatesStatus.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
                magiskCheckUpdatesStatus.setTextColor(colorOK);
                magiskUpdateView.setOnClickListener(view -> builder
                        .setTitle(getString(R.string.repo_install_title, getString(R.string.magisk)))
                        .setMessage(getString(R.string.repo_install_msg, "Magisk-v" + String.valueOf(remoteMagiskVersion)))
                        .setCancelable(true)
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(
                                getActivity(),
                                new DownloadReceiver() {
                                    @Override
                                    public void task(Uri uri) {
                                        new Async.FlashZIP(mContext, uri).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                    }
                                },
                                magiskLink,
                                "Magisk-v" + String.valueOf(remoteMagiskVersion) + ".zip"))
                        .setNegativeButton(R.string.no_thanks, null)
                        .show());
            }

            if (remoteAppVersionCode > BuildConfig.VERSION_CODE) {
                appCheckUpdatesContainer.setBackgroundColor(colorNeutral);
                appCheckUpdatesIcon.setImageResource(R.drawable.ic_file_download);
                appCheckUpdatesStatus.setText(getString(R.string.app_update_available, remoteAppVersion));
                appCheckUpdatesStatus.setTextColor(colorNeutral);
                appUpdateView.setOnClickListener(view -> builder
                        .setTitle(getString(R.string.update_title, getString(R.string.app_name)))
                        .setMessage(getString(R.string.update_msg, getString(R.string.app_name), remoteAppVersion, appChangelog))
                        .setCancelable(true)
                        .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(getActivity(),
                                new DownloadReceiver() {
                                    @Override
                                    public void task(Uri uri) {
                                        Intent install = new Intent(Intent.ACTION_INSTALL_PACKAGE);
                                        install.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                                        Uri content = FileProvider.getUriForFile(getActivity(), "com.topjohnwu.magisk.provider", new File(uri.getPath()));
                                        install.setData(content);
                                        mContext.startActivity(install);
                                    }
                                },
                                appLink,
                                "MagiskManager-v" + remoteAppVersion + ".apk"))
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

        appCheckUpdatesProgress.setVisibility(View.GONE);
        magiskCheckUpdatesProgress.setVisibility(View.GONE);
        mSwipeRefreshLayout.setRefreshing(false);

        if (magiskVersion == -1) {
            builder
                    .setTitle(R.string.no_magisk_title)
                    .setMessage(R.string.no_magisk_msg)
                    .setCancelable(true)
                    .setPositiveButton(R.string.download_install, (dialogInterface, i) -> Utils.downloadAndReceive(
                            getActivity(),
                            new DownloadReceiver() {
                                @Override
                                public void task(Uri uri) {
                                    new Async.FlashZIP(mContext, uri).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                                }
                            },
                            magiskLink,
                            "Magisk-v" + String.valueOf(remoteMagiskVersion) + ".zip"))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }
}
