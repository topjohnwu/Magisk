package com.topjohnwu.magisk;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
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
import com.topjohnwu.magisk.utils.SafetyNetHelper;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;
import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class StatusFragment extends Fragment {

    public static int remoteAppVersionCode = -1;
    public static double magiskVersion, remoteMagiskVersion = -1;
    public static String magiskVersionString, magiskLink, magiskChangelog, appLink, appChangelog, remoteAppVersion;

    private static int lastSNCheckResult = -1;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.magisk_status_container) View magiskStatusContainer;
    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersionText;
    @BindView(R.id.magisk_update_status) TextView magiskUpdateText;
    @BindView(R.id.magisk_check_updates_progress) ProgressBar magiskCheckUpdatesProgress;

    @BindView(R.id.root_status_container) View rootStatusContainer;
    @BindView(R.id.root_status_icon) ImageView rootStatusIcon;
    @BindView(R.id.root_status) TextView rootStatusText;
    @BindView(R.id.root_info) TextView rootInfoText;

    @BindView(R.id.safetyNet_container) View safetyNetContainer;
    @BindView(R.id.safetyNet_icon) ImageView safetyNetIcon;
    @BindView(R.id.safetyNet_status) TextView safetyNetStatusText;
    @BindView(R.id.safetyNet_check_progress) ProgressBar safetyNetProgress;

    @BindColor(R.color.red500) int colorBad;
    @BindColor(R.color.green500) int colorOK;
    @BindColor(R.color.yellow500) int colorWarn;
    @BindColor(R.color.grey500) int colorNeutral;
    @BindColor(R.color.blue500) int colorInfo;
    @BindColor(android.R.color.transparent) int trans;

    private AlertDialog.Builder builder;

    private SharedPreferences prefs;
    private SharedPreferences.OnSharedPreferenceChangeListener listener;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.status_fragment, container, false);
        ButterKnife.bind(this, v);

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        String theme = prefs.getString("theme", "");
        if (theme.equals("Dark")) {
            builder = new AlertDialog.Builder(getActivity(), R.style.AlertDialog_dh);
        } else {
            builder = new AlertDialog.Builder(getActivity());
        }

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            prefs.edit().putBoolean("update_check_done", false).apply();

            magiskStatusContainer.setBackgroundColor(trans);
            magiskStatusIcon.setImageResource(0);
            magiskUpdateText.setText(R.string.checking_for_updates);
            magiskCheckUpdatesProgress.setVisibility(View.VISIBLE);

            new Async.CheckUpdates(prefs).exec();
            checkSafetyNet();
        });

        listener = (pref, s) -> {
            if (s.equals("update_check_done")) {
                if (pref.getBoolean(s, false)) {
                    Logger.dev("StatusFragment: UI refresh triggered");
                    updateUI();
                    updateCheckUI();
                }
            }
        };

        updateUI();
        if (prefs.getBoolean("update_check_done", false)) {
            updateCheckUI();
        }
        if (lastSNCheckResult == -1) {
            checkSafetyNet();
        } else {
            updateSafetyNetUI();
        }

        if (magiskVersion < 0) {
            builder
                    .setTitle(R.string.no_magisk_title)
                    .setMessage(R.string.no_magisk_msg)
                    .setCancelable(true)
                    .setPositiveButton(R.string.download_install, (dialogInterface, i) -> {
                        ((MainActivity) getActivity()).navigationView.setCheckedItem(R.id.install);
                        FragmentTransaction transaction = getFragmentManager().beginTransaction();
                        transaction.setCustomAnimations(android.R.animator.fade_in, android.R.animator.fade_out);
                        try {
                            transaction.replace(R.id.content_frame, new InstallFragment(), "install").commit();
                        } catch (IllegalStateException ignored) {}
                    })
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }

        return v;
    }

    @Override
    public void onResume() {
        super.onResume();
        prefs.registerOnSharedPreferenceChangeListener(listener);
        getActivity().setTitle(R.string.status);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }

    private void updateUI() {
        int image, color;

        List<String> ret = Shell.sh("getprop magisk.version");
        if (ret.get(0).length() == 0) {
            magiskVersion = -1;
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
            try {
                magiskVersionString = ret.get(0);
                magiskVersion = Double.parseDouble(ret.get(0));
            } catch (NumberFormatException e) {
                // Custom version don't need to receive updates
                magiskVersion = Double.POSITIVE_INFINITY;
            }
            magiskVersionText.setText(getString(R.string.magisk_version, magiskVersionString));
        }

        if (Shell.rootStatus == 1) {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            rootStatusText.setText(R.string.proper_root);
            rootInfoText.setText(Shell.sh("su -v").get(0));

        } else {
            rootInfoText.setText(R.string.root_info_warning);
            if (Shell.rootStatus == 0) {
                color = colorBad;
                image = R.drawable.ic_cancel;
                rootStatusText.setText(R.string.not_rooted);
            } else {
                color = colorNeutral;
                image = R.drawable.ic_help;
                rootStatusText.setText(R.string.root_error);
            }
        }
        rootStatusContainer.setBackgroundColor(color);
        rootStatusText.setTextColor(color);
        rootInfoText.setTextColor(color);
        rootStatusIcon.setImageResource(image);
    }

    private void updateCheckUI() {
        int image, color;

        if (remoteMagiskVersion < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            magiskUpdateText.setText(R.string.cannot_check_updates);
        } else if (remoteMagiskVersion > magiskVersion) {
            color = colorInfo;
            image = R.drawable.ic_update;
            magiskUpdateText.setText(getString(R.string.magisk_update_available, String.valueOf(remoteMagiskVersion)));
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskUpdateText.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
        }

        if (magiskVersion < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
        }
        magiskStatusContainer.setBackgroundColor(color);
        magiskVersionText.setTextColor(color);
        magiskUpdateText.setTextColor(color);
        magiskStatusIcon.setImageResource(image);

        magiskCheckUpdatesProgress.setVisibility(View.GONE);
        mSwipeRefreshLayout.setRefreshing(false);
    }

    private void updateSafetyNetUI() {
        int image, color;
        safetyNetProgress.setVisibility(View.GONE);
        switch (lastSNCheckResult) {
            case -1:
                color = colorNeutral;
                image = R.drawable.ic_help;
                safetyNetStatusText.setText(R.string.safetyNet_error);
                break;
            case 0:
                color = colorBad;
                image = R.drawable.ic_cancel;
                safetyNetStatusText.setText(R.string.safetyNet_fail);
                break;
            case 1:
            default:
                color = colorOK;
                image = R.drawable.ic_check_circle;
                safetyNetStatusText.setText(R.string.safetyNet_pass);
                break;
        }
        safetyNetContainer.setBackgroundColor(color);
        safetyNetStatusText.setTextColor(color);
        safetyNetIcon.setImageResource(image);
    }

    private void checkSafetyNet() {
        safetyNetProgress.setVisibility(View.VISIBLE);
        safetyNetContainer.setBackgroundColor(trans);
        safetyNetIcon.setImageResource(0);
        safetyNetStatusText.setText(R.string.checking_safetyNet_status);
        new SafetyNetHelper(getActivity()) {
            @Override
            public void handleResults(int i) {
                lastSNCheckResult = i;
                updateSafetyNetUI();
            }
        }.requestTest();
    }
}

