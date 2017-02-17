package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.widget.SwipeRefreshLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class StatusFragment extends Fragment implements CallbackEvent.Listener<Void> {

    private static boolean noDialog = false;

    private Unbinder unbinder;
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

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_status, container, false);
        unbinder = ButterKnife.bind(this, v);

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

            getApplication().safetyNetDone.isTriggered = false;
            noDialog = false;

            updateUI();
            new CheckUpdates(getActivity()).exec();
        });

        safetyNetContainer.setOnClickListener(view -> {
            safetyNetProgress.setVisibility(View.VISIBLE);
            safetyNetContainer.setBackgroundColor(trans);
            safetyNetIcon.setImageResource(0);
            safetyNetStatusText.setText(R.string.checking_safetyNet_status);
            Utils.checkSafetyNet(getApplication());
        });

        magiskStatusContainer.setOnClickListener(view -> {
            ((MainActivity) getActivity()).navigationView.setCheckedItem(R.id.install);
            FragmentTransaction transaction = getFragmentManager().beginTransaction();
            transaction.setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out);
            try {
                transaction.replace(R.id.content_frame, new InstallFragment(), "install").commit();
            } catch (IllegalStateException ignored) {}
        });

        if (getApplication().magiskVersion < 0 && Shell.rootAccess() && !noDialog) {
            noDialog = true;
            new AlertDialogBuilder(getActivity())
                    .setTitle(R.string.no_magisk_title)
                    .setMessage(R.string.no_magisk_msg)
                    .setCancelable(true)
                    .setPositiveButton(R.string.goto_install, (dialogInterface, i) -> {
                        ((MainActivity) getActivity()).navigationView.setCheckedItem(R.id.install);
                        FragmentTransaction transaction = getFragmentManager().beginTransaction();
                        transaction.setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out);
                        try {
                            transaction.replace(R.id.content_frame, new InstallFragment(), "install").commit();
                        } catch (IllegalStateException ignored) {}
                    })
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }

        updateUI();

        if (getApplication().updateCheckDone.isTriggered)
            updateCheckUI();
        if (getApplication().safetyNetDone.isTriggered)
            updateSafetyNetUI();

        return v;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        if (event == getApplication().updateCheckDone) {
            Logger.dev("StatusFragment: Update Check UI refresh triggered");
            updateCheckUI();
        } else if (event == getApplication().safetyNetDone) {
            Logger.dev("StatusFragment: SafetyNet UI refresh triggered");
            updateSafetyNetUI();
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        getApplication().updateCheckDone.register(this);
        getApplication().safetyNetDone.register(this);
        getActivity().setTitle(R.string.status);
    }

    @Override
    public void onStop() {
        getApplication().updateCheckDone.unRegister(this);
        getApplication().safetyNetDone.unRegister(this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    private void updateUI() {
        int image, color;

        getApplication().updateMagiskInfo();

        if (getApplication().magiskVersion < 0) {
            magiskVersionText.setText(R.string.magisk_version_error);
        } else if (getApplication().disabled) {
            magiskVersionText.setText(getString(R.string.magisk_version_disable, getApplication().magiskVersionString));
        } else {
            magiskVersionText.setText(getString(R.string.magisk_version, getApplication().magiskVersionString));
        }

        switch (Shell.rootStatus) {
            case 0:
                color = colorBad;
                image = R.drawable.ic_cancel;
                rootStatusText.setText(R.string.not_rooted);
                rootInfoText.setText(R.string.root_info_warning);
                break;
            case 1:
                if (getApplication().suVersion != null) {
                    color = colorOK;
                    image = R.drawable.ic_check_circle;
                    rootStatusText.setText(R.string.proper_root);
                    rootInfoText.setText(getApplication().suVersion);
                    break;
                }
            case -1:
            default:
                color = colorNeutral;
                image = R.drawable.ic_help;
                rootStatusText.setText(R.string.root_error);
                rootInfoText.setText(R.string.root_info_warning);
        }
        rootStatusContainer.setBackgroundColor(color);
        rootStatusText.setTextColor(color);
        rootInfoText.setTextColor(color);
        rootStatusIcon.setImageResource(image);
    }

    private void updateCheckUI() {
        int image, color;

        if (getApplication().remoteMagiskVersion < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            magiskUpdateText.setText(R.string.cannot_check_updates);
        } else if (getApplication().remoteMagiskVersion > getApplication().magiskVersion) {
            color = colorInfo;
            image = R.drawable.ic_update;
            magiskUpdateText.setText(getString(R.string.magisk_update_available, getApplication().remoteMagiskVersion));
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskUpdateText.setText(getString(R.string.up_to_date, getString(R.string.magisk)));
        }

        if (getApplication().magiskVersion < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
        } else if (getApplication().disabled) {
            color = colorNeutral;
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
        switch (getApplication().SNCheckResult) {
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

