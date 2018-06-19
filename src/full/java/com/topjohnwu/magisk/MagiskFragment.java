package com.topjohnwu.magisk;

import android.app.NotificationManager;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.annotation.StringRes;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.CardView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.asyncs.CheckSafetyNet;
import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.components.ExpandableView;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.ISafetyNetHelper;
import com.topjohnwu.magisk.utils.ShowUI;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class MagiskFragment extends Fragment
        implements Topic.Subscriber, SwipeRefreshLayout.OnRefreshListener, ExpandableView {

    private Container expandableContainer = new Container();

    private MagiskManager mm;
    private Unbinder unbinder;
    private static boolean shownDialog = false;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.magisk_update) RelativeLayout magiskUpdate;
    @BindView(R.id.magisk_update_icon) ImageView magiskUpdateIcon;
    @BindView(R.id.magisk_update_status) TextView magiskUpdateText;
    @BindView(R.id.magisk_update_progress) ProgressBar magiskUpdateProgress;
    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersionText;

    @BindView(R.id.safetyNet_card) CardView safetyNetCard;
    @BindView(R.id.safetyNet_refresh) ImageView safetyNetRefreshIcon;
    @BindView(R.id.safetyNet_status) TextView safetyNetStatusText;
    @BindView(R.id.safetyNet_check_progress) ProgressBar safetyNetProgress;
    @BindView(R.id.expand_layout) LinearLayout expandLayout;
    @BindView(R.id.cts_status_icon) ImageView ctsStatusIcon;
    @BindView(R.id.cts_status) TextView ctsStatusText;
    @BindView(R.id.basic_status_icon) ImageView basicStatusIcon;
    @BindView(R.id.basic_status) TextView basicStatusText;

    @BindView(R.id.install_option_card) CardView installOptionCard;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;
    @BindView(R.id.install_button) CardView installButton;
    @BindView(R.id.install_text) TextView installText;
    @BindView(R.id.uninstall_button) CardView uninstallButton;

    @BindColor(R.color.red500) int colorBad;
    @BindColor(R.color.green500) int colorOK;
    @BindColor(R.color.yellow500) int colorWarn;
    @BindColor(R.color.grey500) int colorNeutral;
    @BindColor(R.color.blue500) int colorInfo;

    @OnClick(R.id.safetyNet_title)
    void safetyNet() {
        Runnable task = () -> {
            safetyNetProgress.setVisibility(View.VISIBLE);
            safetyNetRefreshIcon.setVisibility(View.GONE);
            safetyNetStatusText.setText(R.string.checking_safetyNet_status);
            new CheckSafetyNet(getActivity()).exec();
            collapse();
        };
        if (!TextUtils.equals(mm.getPackageName(), Const.ORIG_PKG_NAME)) {
            new AlertDialogBuilder(getActivity())
                    .setTitle(R.string.cannot_check_sn_title)
                    .setMessage(R.string.cannot_check_sn_notice)
                    .setCancelable(true)
                    .setPositiveButton(R.string.ok, null)
                    .show();
        } else if (!CheckSafetyNet.dexPath.exists()) {
            // Show dialog
            new AlertDialogBuilder(getActivity())
                    .setTitle(R.string.proprietary_title)
                    .setMessage(R.string.proprietary_notice)
                    .setCancelable(true)
                    .setPositiveButton(R.string.yes, (d, i) -> task.run())
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        } else {
            task.run();
        }

    }

    @OnClick(R.id.install_button)
    void install() {
        shownDialog = true;

        // Show Manager update first
        if (mm.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
            ShowUI.managerInstallDialog(getActivity());
            return;
        }

        ((NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE)).cancelAll();
        ShowUI.magiskInstallDialog(getActivity());
    }

    @OnClick(R.id.uninstall_button)
    void uninstall() {
        ShowUI.uninstallDialog(getActivity());
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_magisk, container, false);
        unbinder = ButterKnife.bind(this, v);
        getActivity().setTitle(R.string.magisk);

        mm = getApplication();

        expandableContainer.expandLayout = expandLayout;
        setupExpandable();

        keepVerityChkbox.setChecked(mm.keepVerity);
        keepVerityChkbox.setOnCheckedChangeListener((view, checked) -> mm.keepVerity = checked);
        keepEncChkbox.setChecked(mm.keepEnc);
        keepEncChkbox.setOnCheckedChangeListener((view, checked) -> mm.keepEnc = checked);

        mSwipeRefreshLayout.setOnRefreshListener(this);
        updateUI();

        return v;
    }

    @Override
    public void onRefresh() {
        mm.loadMagiskInfo();
        updateUI();

        magiskUpdateText.setText(R.string.checking_for_updates);
        magiskUpdateProgress.setVisibility(View.VISIBLE);
        magiskUpdateIcon.setVisibility(View.GONE);

        safetyNetStatusText.setText(R.string.safetyNet_check_text);

        mm.safetyNetDone.reset();
        mm.updateCheckDone.reset();
        mm.remoteMagiskVersionString = null;
        mm.remoteMagiskVersionCode = -1;
        collapse();

        shownDialog = false;

        // Trigger state check
        if (Utils.checkNetworkStatus()) {
            new CheckUpdates().exec();
        } else {
            mSwipeRefreshLayout.setRefreshing(false);
        }
    }

    @Override
    public void onTopicPublished(Topic topic) {
        if (topic == mm.updateCheckDone) {
            updateCheckUI();
        } else if (topic == mm.safetyNetDone) {
            updateSafetyNetUI((int) topic.getResults()[0]);
        }
    }

    @Override
    public Topic[] getSubscription() {
        return new Topic[] { mm.updateCheckDone, mm.safetyNetDone };
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    @Override
    public Container getContainer() {
        return expandableContainer;
    }

    private void updateUI() {
        ((MainActivity) getActivity()).checkHideSection();

        boolean hasNetwork = Utils.checkNetworkStatus();
        boolean hasRoot = Shell.rootAccess();
        boolean isUpToDate = mm.magiskVersionCode > Const.MAGISK_VER.UNIFIED;

        magiskUpdate.setVisibility(hasNetwork ? View.VISIBLE : View.GONE);
        safetyNetCard.setVisibility(hasNetwork ? View.VISIBLE : View.GONE);
        installOptionCard.setVisibility(hasNetwork ? View.VISIBLE : View.GONE);
        uninstallButton.setVisibility(isUpToDate && hasRoot ? View.VISIBLE : View.GONE);

        int image, color;

        if (mm.magiskVersionCode < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskVersionText.setText(getString(R.string.current_magisk_title, "v" + mm.magiskVersionString));
        }

        magiskStatusIcon.setImageResource(image);
        magiskStatusIcon.setColorFilter(color);
    }

    private void updateCheckUI() {
        int image, color;

        if (mm.remoteMagiskVersionCode < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            magiskUpdateText.setText(R.string.invalid_update_channel);
            installButton.setVisibility(View.GONE);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskUpdateText.setText(getString(R.string.install_magisk_title, "v" + mm.remoteMagiskVersionString));
            installButton.setVisibility(View.VISIBLE);
            if (mm.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
                installText.setText(getString(R.string.update, getString(R.string.app_name)));
            } else if (mm.magiskVersionCode > 0 && mm.remoteMagiskVersionCode > mm.magiskVersionCode) {
                installText.setText(getString(R.string.update, getString(R.string.magisk)));
            } else {
                installText.setText(R.string.install);
            }
        }

        magiskUpdateIcon.setImageResource(image);
        magiskUpdateIcon.setColorFilter(color);
        magiskUpdateIcon.setVisibility(View.VISIBLE);

        magiskUpdateProgress.setVisibility(View.GONE);
        mSwipeRefreshLayout.setRefreshing(false);

        if (!shownDialog) {
            if (mm.remoteMagiskVersionCode > mm.magiskVersionCode
                    || mm.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
                install();
            } else if (mm.remoteMagiskVersionCode >= Const.MAGISK_VER.FIX_ENV &&
                    !ShellUtils.fastCmdResult("env_check")) {
                ShowUI.envFixDialog(getActivity());
            }
        }
    }

    private void updateSafetyNetUI(int response) {
        safetyNetProgress.setVisibility(View.GONE);
        safetyNetRefreshIcon.setVisibility(View.VISIBLE);
        if ((response & 0x0F) == 0) {
            safetyNetStatusText.setText(R.string.safetyNet_check_success);

            boolean b;
            b = (response & ISafetyNetHelper.CTS_PASS) != 0;
            ctsStatusText.setText("ctsProfile: " + b);
            ctsStatusIcon.setImageResource(b ? R.drawable.ic_check_circle : R.drawable.ic_cancel);
            ctsStatusIcon.setColorFilter(b ? colorOK : colorBad);

            b = (response & ISafetyNetHelper.BASIC_PASS) != 0;
            basicStatusText.setText("basicIntegrity: " + b);
            basicStatusIcon.setImageResource(b ? R.drawable.ic_check_circle : R.drawable.ic_cancel);
            basicStatusIcon.setColorFilter(b ? colorOK : colorBad);

            expand();
        } else {
            @StringRes int resid;
            switch (response) {
                case ISafetyNetHelper.CAUSE_SERVICE_DISCONNECTED:
                    resid = R.string.safetyNet_network_loss;
                    break;
                case ISafetyNetHelper.CAUSE_NETWORK_LOST:
                    resid = R.string.safetyNet_service_disconnected;
                    break;
                case ISafetyNetHelper.RESPONSE_ERR:
                    resid = R.string.safetyNet_res_invalid;
                    break;
                case ISafetyNetHelper.CONNECTION_FAIL:
                default:
                    resid = R.string.safetyNet_api_error;
                    break;
            }
            safetyNetStatusText.setText(resid);
        }
    }
}

