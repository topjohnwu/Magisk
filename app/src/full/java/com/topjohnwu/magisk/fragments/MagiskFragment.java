package com.topjohnwu.magisk.fragments;

import android.app.NotificationManager;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MainActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.CheckSafetyNet;
import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.components.CustomAlertDialog;
import com.topjohnwu.magisk.components.EnvFixDialog;
import com.topjohnwu.magisk.components.ExpandableView;
import com.topjohnwu.magisk.components.MagiskInstallDialog;
import com.topjohnwu.magisk.components.ManagerInstallDialog;
import com.topjohnwu.magisk.components.UninstallDialog;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.ISafetyNetHelper;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.cardview.widget.CardView;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;
import butterknife.BindColor;
import butterknife.BindView;
import butterknife.OnClick;

public class MagiskFragment extends BaseFragment
        implements SwipeRefreshLayout.OnRefreshListener, ExpandableView, Topic.Subscriber {

    private Container expandableContainer = new Container();
    private static boolean shownDialog = false;

    @BindView(R.id.swipeRefreshLayout) public SwipeRefreshLayout mSwipeRefreshLayout;

    @BindView(R.id.core_only_notice) CardView coreOnlyNotice;

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
    @BindColor(R.color.green500) int colorNeutral;
    @BindColor(R.color.blue500) int colorInfo;

    @OnClick(R.id.safetyNet_title)
    void safetyNet() {
        Runnable task = () -> {
            safetyNetProgress.setVisibility(View.VISIBLE);
            safetyNetRefreshIcon.setVisibility(View.GONE);
            safetyNetStatusText.setText(R.string.checking_safetyNet_status);
            new CheckSafetyNet(requireActivity()).exec();
            collapse();
        };
        if (!CheckSafetyNet.dexPath.exists()) {
            // Show dialog
            new CustomAlertDialog(requireActivity())
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
        if (Data.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
            new ManagerInstallDialog((BaseActivity) requireActivity()).show();
            return;
        }

        ((NotificationManager) mm.getSystemService(Context.NOTIFICATION_SERVICE)).cancelAll();
        new MagiskInstallDialog((BaseActivity) getActivity()).show();
    }

    @OnClick(R.id.uninstall_button)
    void uninstall() {
        new UninstallDialog(requireActivity()).show();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_magisk, container, false);
        unbinder = new MagiskFragment_ViewBinding(this, v);
        requireActivity().setTitle(R.string.magisk);

        expandableContainer.expandLayout = expandLayout;
        setupExpandable();

        keepVerityChkbox.setChecked(Data.keepVerity);
        keepVerityChkbox.setOnCheckedChangeListener((view, checked) -> Data.keepVerity = checked);
        keepEncChkbox.setChecked(Data.keepEnc);
        keepEncChkbox.setOnCheckedChangeListener((view, checked) -> Data.keepEnc = checked);

        mSwipeRefreshLayout.setOnRefreshListener(this);
        updateUI();

        return v;
    }

    @Override
    public void onRefresh() {
        Data.loadMagiskInfo();
        updateUI();

        magiskUpdateText.setText(R.string.checking_for_updates);
        magiskUpdateProgress.setVisibility(View.VISIBLE);
        magiskUpdateIcon.setVisibility(View.GONE);

        safetyNetStatusText.setText(R.string.safetyNet_check_text);

        Topic.reset(getSubscribedTopics());
        Data.remoteMagiskVersionString = null;
        Data.remoteMagiskVersionCode = -1;
        collapse();

        shownDialog = false;

        // Trigger state check
        if (Download.checkNetworkStatus(mm)) {
            CheckUpdates.check();
        } else {
            mSwipeRefreshLayout.setRefreshing(false);
        }
    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.SNET_CHECK_DONE, Topic.UPDATE_CHECK_DONE};
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        switch (topic) {
            case Topic.SNET_CHECK_DONE:
                updateSafetyNetUI((int) result[0]);
                break;
            case Topic.UPDATE_CHECK_DONE:
                updateCheckUI();
                break;
        }
    }

    @Override
    public Container getContainer() {
        return expandableContainer;
    }

    private boolean hasGms() {
        PackageManager pm = mm.getPackageManager();
        PackageInfo info;
        try {
            info = pm.getPackageInfo("com.google.android.gms", 0);
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
        return info.applicationInfo.enabled;
    }

    private void updateUI() {
        ((MainActivity) requireActivity()).checkHideSection();

        boolean hasNetwork = Download.checkNetworkStatus(mm);
        boolean hasRoot = Shell.rootAccess();

        magiskUpdate.setVisibility(hasNetwork ? View.VISIBLE : View.GONE);
        installOptionCard.setVisibility(hasNetwork ? View.VISIBLE : View.GONE);
        uninstallButton.setVisibility(hasRoot ? View.VISIBLE : View.GONE);
        coreOnlyNotice.setVisibility(mm.prefs.getBoolean(Const.Key.COREONLY, false) ? View.VISIBLE : View.GONE);

        int image, color;

        if (Data.magiskVersionCode < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
            magiskVersionText.setText(R.string.magisk_version_error);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskVersionText.setText(getString(R.string.current_magisk_title, "v" + Data.magiskVersionString));
        }

        magiskStatusIcon.setImageResource(image);
        magiskStatusIcon.setColorFilter(color);
    }

    private void updateCheckUI() {
        int image, color;

        safetyNetCard.setVisibility(hasGms() ? View.VISIBLE : View.GONE);

        if (Data.remoteMagiskVersionCode < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            magiskUpdateText.setText(R.string.invalid_update_channel);
            installButton.setVisibility(View.GONE);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            magiskUpdateText.setText(getString(R.string.install_magisk_title, "v" + Data.remoteMagiskVersionString));
            installButton.setVisibility(View.VISIBLE);
            if (Data.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
                installText.setText(getString(R.string.update, getString(R.string.app_name)));
            } else if (Data.magiskVersionCode > 0 && Data.remoteMagiskVersionCode > Data.magiskVersionCode) {
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
            if (Data.remoteMagiskVersionCode > Data.magiskVersionCode
                    || Data.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
                install();
            } else if (!ShellUtils.fastCmdResult("env_check")) {
                new EnvFixDialog(requireActivity()).show();
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

