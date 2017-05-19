package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.Nullable;
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
import butterknife.OnClick;
import butterknife.Unbinder;

public class StatusFragment extends Fragment implements CallbackEvent.Listener<Void> {

    private static boolean noDialog = false;

    private Unbinder unbinder;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.magisk_update_icon) ImageView magiskUpdateIcon;
    @BindView(R.id.magisk_update_status) TextView magiskUpdateText;
    @BindView(R.id.magisk_check_updates_progress) ProgressBar magiskCheckUpdatesProgress;

    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersionText;

    @BindView(R.id.root_status_icon) ImageView rootStatusIcon;
    @BindView(R.id.root_status) TextView rootStatusText;

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

    @OnClick(R.id.safetyNet_container)
    public void safetyNet() {
        safetyNetProgress.setVisibility(View.VISIBLE);
        safetyNetContainer.setBackgroundColor(trans);
        safetyNetIcon.setImageResource(0);
        safetyNetStatusText.setText(R.string.checking_safetyNet_status);
        Utils.checkSafetyNet(magiskManager);
    }

    public void gotoInstall() {
        if (magiskManager.remoteMagiskVersionCode > 0) {
            ((MainActivity) getActivity()).navigate(R.id.install);
        }
    }

    private int defaultColor;
    private MagiskManager magiskManager;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_status, container, false);
        unbinder = ButterKnife.bind(this, v);
        magiskManager = getApplication();

        defaultColor = magiskUpdateText.getCurrentTextColor();

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            magiskUpdateText.setText(R.string.checking_for_updates);
            magiskCheckUpdatesProgress.setVisibility(View.VISIBLE);
            magiskUpdateIcon.setVisibility(View.GONE);

            safetyNetProgress.setVisibility(View.GONE);
            safetyNetContainer.setBackgroundColor(colorNeutral);
            safetyNetIcon.setImageResource(R.drawable.ic_safetynet);
            safetyNetStatusText.setText(R.string.safetyNet_check_text);
            safetyNetStatusText.setTextColor(defaultColor);

            magiskManager.safetyNetDone.isTriggered = false;
            noDialog = false;

            updateUI();
            new CheckUpdates(getActivity()).exec();
        });

        if (magiskManager.magiskVersionCode < 0 && Shell.rootAccess() && !noDialog) {
            noDialog = true;
            new AlertDialogBuilder(getActivity())
                    .setTitle(R.string.no_magisk_title)
                    .setMessage(R.string.no_magisk_msg)
                    .setCancelable(true)
                    .setPositiveButton(R.string.goto_install, (d, i) -> gotoInstall())
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }

        updateUI();

        if (magiskManager.updateCheckDone.isTriggered)
            updateCheckUI();
        if (magiskManager.safetyNetDone.isTriggered)
            updateSafetyNetUI();

        return v;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        if (event == magiskManager.updateCheckDone) {
            Logger.dev("StatusFragment: Update Check UI refresh triggered");
            updateCheckUI();
        } else if (event == magiskManager.safetyNetDone) {
            Logger.dev("StatusFragment: SafetyNet UI refresh triggered");
            updateSafetyNetUI();
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        magiskManager.updateCheckDone.register(this);
        magiskManager.safetyNetDone.register(this);
        getActivity().setTitle(R.string.status);
    }

    @Override
    public void onStop() {
        magiskManager.updateCheckDone.unRegister(this);
        magiskManager.safetyNetDone.unRegister(this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    private void updateUI() {
        int image, color;

        magiskManager.updateMagiskInfo();

        if (magiskManager.magiskVersionCode < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskVersionText.setText(getString(R.string.current_magisk_title, "v" + magiskManager.magiskVersionString));
        }

        magiskStatusIcon.setImageResource(image);
        magiskStatusIcon.setColorFilter(color);

        switch (Shell.rootStatus) {
            case 0:
                color = colorBad;
                image = R.drawable.ic_cancel;
                rootStatusText.setText(R.string.not_rooted);
                break;
            case 1:
                if (magiskManager.suVersion != null) {
                    color = colorOK;
                    image = R.drawable.ic_check_circle;
                    rootStatusText.setText(magiskManager.suVersion);
                    break;
                }
            case -1:
            default:
                color = colorNeutral;
                image = R.drawable.ic_help;
                rootStatusText.setText(R.string.root_error);
        }

        rootStatusIcon.setImageResource(image);
        rootStatusIcon.setColorFilter(color);
    }

    private void updateCheckUI() {
        int image, color;

        if (magiskManager.remoteMagiskVersionCode < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            magiskUpdateText.setText(R.string.cannot_check_updates);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskUpdateText.setText(getString(R.string.install_magisk_title, "v" + magiskManager.remoteMagiskVersionString));
        }

        magiskUpdateIcon.setImageResource(image);
        magiskUpdateIcon.setColorFilter(color);
        magiskUpdateIcon.setVisibility(View.VISIBLE);

        magiskCheckUpdatesProgress.setVisibility(View.GONE);
        mSwipeRefreshLayout.setRefreshing(false);
    }

    private void updateSafetyNetUI() {
        int image, color;
        safetyNetProgress.setVisibility(View.GONE);
        switch (magiskManager.SNCheckResult) {
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

