package com.topjohnwu.magisk;

import android.app.AlertDialog;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class StatusFragment extends Fragment implements CallbackHandler.EventListener {

    public static double magiskVersion, remoteMagiskVersion = -1;
    public static String magiskVersionString = "(none)", magiskLink, releaseNoteLink;
    public static int SNCheckResult = -1;

    private static boolean noDialog = false;

    public static final CallbackHandler.Event updateCheckDone = new CallbackHandler.Event();
    public static final CallbackHandler.Event safetyNetDone = new CallbackHandler.Event();

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
    int defaultColor;

    static {
        checkMagiskInfo();
    }

    private AlertDialog updateMagisk;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.status_fragment, container, false);
        ButterKnife.bind(this, v);

        defaultColor = magiskUpdateText.getCurrentTextColor();

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            magiskStatusContainer.setBackgroundColor(trans);
            magiskStatusIcon.setImageResource(0);
            magiskUpdateText.setText(R.string.checking_for_updates);
            magiskCheckUpdatesProgress.setVisibility(View.VISIBLE);
            magiskUpdateText.setTextColor(defaultColor);

            safetyNetProgress.setVisibility(View.GONE);
            safetyNetContainer.setBackgroundColor(colorNeutral);
            safetyNetIcon.setImageResource(R.drawable.ic_safetynet);
            safetyNetStatusText.setText(R.string.safetyNet_check_text);
            safetyNetStatusText.setTextColor(defaultColor);

            safetyNetDone.isTriggered = false;
            noDialog = false;

            updateUI();
            new Async.CheckUpdates().exec();
        });

        safetyNetContainer.setOnClickListener(view -> {
            safetyNetProgress.setVisibility(View.VISIBLE);
            safetyNetContainer.setBackgroundColor(trans);
            safetyNetIcon.setImageResource(0);
            safetyNetStatusText.setText(R.string.checking_safetyNet_status);
            Async.checkSafetyNet(getActivity());
        });

        if (magiskVersion < 0 && Shell.rootAccess() && !noDialog) {
            noDialog = true;
            Utils.getAlertDialogBuilder(getActivity())
                    .setTitle(R.string.no_magisk_title)
                    .setMessage(R.string.no_magisk_msg)
                    .setCancelable(true)
                    .setPositiveButton(R.string.goto_install, (dialogInterface, i) -> {
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

        updateUI();

        return v;
    }

    @Override
    public void onTrigger(CallbackHandler.Event event) {
        if (event == updateCheckDone) {
            Logger.dev("StatusFragment: Update Check UI refresh triggered");
            updateCheckUI();
        } else if (event == safetyNetDone) {
            Logger.dev("StatusFragment: SafetyNet UI refresh triggered");
            updateSafetyNetUI();
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        CallbackHandler.register(updateCheckDone, this);
        CallbackHandler.register(safetyNetDone, this);
        if (updateCheckDone.isTriggered) {
            updateCheckUI();
        }
        if (safetyNetDone.isTriggered) {
            updateSafetyNetUI();
        }
        getActivity().setTitle(R.string.status);
    }

    @Override
    public void onStop() {
        CallbackHandler.unRegister(updateCheckDone, this);
        CallbackHandler.unRegister(safetyNetDone, this);
        super.onStop();
    }

    private static void checkMagiskInfo() {
        List<String> ret = Shell.sh("getprop magisk.version");
        if (ret.get(0).length() == 0) {
            magiskVersion = -1;
        } else {
            try {
                magiskVersionString = ret.get(0);
                magiskVersion = Double.parseDouble(ret.get(0));
            } catch (NumberFormatException e) {
                // Custom version don't need to receive updates
                magiskVersion = Double.POSITIVE_INFINITY;
            }
        }
    }

    private void updateUI() {
        int image, color;

        checkMagiskInfo();

        if (magiskVersion < 0) {
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
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
            magiskUpdateText.setText(getString(R.string.magisk_update_available, remoteMagiskVersion));
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

        updateMagisk = Utils.getAlertDialogBuilder(getActivity())
                .setTitle(R.string.magisk_update_title)
                .setMessage(getString(R.string.magisk_update_message, remoteMagiskVersion))
                .setCancelable(true)
                .setPositiveButton(R.string.goto_install, (dialogInterface, i) -> {
                    ((MainActivity) getActivity()).navigationView.setCheckedItem(R.id.install);
                    FragmentTransaction transaction = getFragmentManager().beginTransaction();
                    transaction.setCustomAnimations(android.R.animator.fade_in, android.R.animator.fade_out);
                    try {
                        transaction.replace(R.id.content_frame, new InstallFragment(), "install").commit();
                    } catch (IllegalStateException ignored) {}
                })
                .setNeutralButton(R.string.check_release_notes, (dialog, which) -> {
                    getActivity().startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(releaseNoteLink)));
                })
                .setNegativeButton(R.string.no_thanks, null)
                .create();

        if (magiskVersion < remoteMagiskVersion && Shell.rootAccess()) {
            magiskStatusContainer.setOnClickListener(view -> updateMagisk.show());
            if (!noDialog) {
                noDialog = true;
                updateMagisk.show();
            }
        }
    }

    private void updateSafetyNetUI() {
        int image, color;
        safetyNetProgress.setVisibility(View.GONE);
        switch (SNCheckResult) {
            case -3:
                color = colorNeutral;
                image = R.drawable.ic_help;
                safetyNetStatusText.setText(R.string.safetyNet_connection_suspended);
                break;
            case -2:
                color = colorNeutral;
                image = R.drawable.ic_help;
                safetyNetStatusText.setText(R.string.safetyNet_connection_failed);
                break;
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
}

