package com.topjohnwu.magisk.fragments;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.LinearLayout;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.MainActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.dialogs.EnvFixDialog;
import com.topjohnwu.magisk.dialogs.MagiskInstallDialog;
import com.topjohnwu.magisk.dialogs.ManagerInstallDialog;
import com.topjohnwu.magisk.dialogs.UninstallDialog;
import com.topjohnwu.magisk.tasks.CheckUpdates;
import com.topjohnwu.magisk.uicomponents.SafetyNet;
import com.topjohnwu.magisk.uicomponents.UpdateCardHolder;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.net.Networking;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.util.Locale;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.cardview.widget.CardView;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;
import androidx.transition.ChangeBounds;
import androidx.transition.Fade;
import androidx.transition.Transition;
import androidx.transition.TransitionManager;
import androidx.transition.TransitionSet;
import butterknife.BindColor;
import butterknife.BindView;
import butterknife.OnClick;

public class MagiskFragment extends BaseFragment
        implements SwipeRefreshLayout.OnRefreshListener, Topic.Subscriber {

    private static boolean shownDialog = false;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.linearLayout) LinearLayout root;

    @BindView(R.id.install_option_card) CardView installOptionCard;
    @BindView(R.id.keep_force_enc) CheckBox keepEncChkbox;
    @BindView(R.id.keep_verity) CheckBox keepVerityChkbox;
    @BindView(R.id.uninstall_button) CardView uninstallButton;

    @BindColor(R.color.red500) int colorBad;
    @BindColor(R.color.green500) int colorOK;
    @BindColor(R.color.yellow500) int colorWarn;
    @BindColor(R.color.green500) int colorNeutral;
    @BindColor(R.color.blue500) int colorInfo;

    private UpdateCardHolder magisk;
    private UpdateCardHolder manager;
    private SafetyNet safetyNet;
    private Transition transition;

    private void magiskInstall(View v) {
        // Show Manager update first
        if (Config.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
            new ManagerInstallDialog(requireActivity()).show();
            return;
        }
        new MagiskInstallDialog((BaseActivity) requireActivity()).show();
    }

    private void managerInstall(View v) {
        new ManagerInstallDialog(requireActivity()).show();
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

        safetyNet = new SafetyNet(v);
        magisk = new UpdateCardHolder(inflater, root);
        manager = new UpdateCardHolder(inflater, root);
        root.addView(magisk.itemView, 0);
        root.addView(manager.itemView, 1);

        keepVerityChkbox.setChecked(Config.keepVerity);
        keepVerityChkbox.setOnCheckedChangeListener((view, checked) -> Config.keepVerity = checked);
        keepEncChkbox.setChecked(Config.keepEnc);
        keepEncChkbox.setOnCheckedChangeListener((view, checked) -> Config.keepEnc = checked);

        mSwipeRefreshLayout.setOnRefreshListener(this);

        magisk.install.setOnClickListener(this::magiskInstall);
        manager.install.setOnClickListener(this::managerInstall);
        if (Config.get(Config.Key.COREONLY)) {
            magisk.additional.setText(R.string.core_only_enabled);
            magisk.additional.setVisibility(View.VISIBLE);
        }
        if (!app.getPackageName().equals(BuildConfig.APPLICATION_ID)) {
            manager.additional.setText("(" + app.getPackageName() +  ")");
            manager.additional.setVisibility(View.VISIBLE);
        }

        transition = new TransitionSet()
                .setOrdering(TransitionSet.ORDERING_TOGETHER)
                .addTransition(new Fade(Fade.OUT))
                .addTransition(new ChangeBounds())
                .addTransition(new Fade(Fade.IN));

        updateUI();
        return v;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        safetyNet.unbinder.unbind();
        magisk.unbinder.unbind();
        manager.unbinder.unbind();
    }

    @Override
    public void onRefresh() {
        mSwipeRefreshLayout.setRefreshing(false);
        TransitionManager.beginDelayedTransition(root, transition);
        safetyNet.reset();
        magisk.reset();
        manager.reset();

        Config.loadMagiskInfo();
        updateUI();

        Topic.reset(getSubscribedTopics());
        Config.remoteMagiskVersionString = null;
        Config.remoteMagiskVersionCode = -1;

        shownDialog = false;

        // Trigger state check
        if (Networking.checkNetworkStatus(app)) {
            CheckUpdates.check();
        }
    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.UPDATE_CHECK_DONE};
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        updateCheckUI();
    }

    private boolean hasGms() {
        PackageManager pm = app.getPackageManager();
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
        int image, color;
        String status;
        if (Config.magiskVersionCode < 0) {
            color = colorBad;
            image = R.drawable.ic_cancel;
            status = getString(R.string.magisk_version_error);
            magisk.status.setText(status);
            magisk.currentVersion.setVisibility(View.GONE);
        } else {
            color = colorOK;
            image = R.drawable.ic_check_circle;
            status = getString(R.string.magisk);
            magisk.currentVersion.setText(getString(R.string.current_installed,
                    String.format(Locale.US, "v%s (%d)",
                            Config.magiskVersionString, Config.magiskVersionCode)));
        }
        magisk.statusIcon.setColorFilter(color);
        magisk.statusIcon.setImageResource(image);

        manager.statusIcon.setColorFilter(colorOK);
        manager.statusIcon.setImageResource(R.drawable.ic_check_circle);
        manager.currentVersion.setText(getString(R.string.current_installed,
                String.format(Locale.US, "v%s (%d)",
                        BuildConfig.VERSION_NAME, BuildConfig.VERSION_CODE)));

        if (!Networking.checkNetworkStatus(app)) {
            // No network, updateCheckUI will not be triggered
            magisk.status.setText(status);
            manager.status.setText(R.string.app_name);
            magisk.setValid(false);
            manager.setValid(false);
        }
    }

    private void updateCheckUI() {
        int image, color;
        String status;

        if (Config.remoteMagiskVersionCode < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            status = getString(R.string.invalid_update_channel);
        } else {
            magisk.latestVersion.setText(getString(R.string.latest_version,
                    String.format(Locale.US, "v%s (%d)",
                            Config.remoteMagiskVersionString, Config.remoteMagiskVersionCode)));
            if (Config.remoteMagiskVersionCode > Config.magiskVersionCode) {
                color = colorInfo;
                image = R.drawable.ic_update;
                status = getString(R.string.magisk_update_title);
                magisk.install.setText(R.string.update);
            } else {
                color = colorOK;
                image = R.drawable.ic_check_circle;
                status = getString(R.string.magisk_up_to_date);
                magisk.install.setText(R.string.install);
            }
        }
        if (Config.magiskVersionCode > 0) {
            // Only override status if Magisk is installed
            magisk.statusIcon.setImageResource(image);
            magisk.statusIcon.setColorFilter(color);
            magisk.status.setText(status);
        }

        if (Config.remoteManagerVersionCode < 0) {
            color = colorNeutral;
            image = R.drawable.ic_help;
            status = getString(R.string.invalid_update_channel);
        } else {
            manager.latestVersion.setText(getString(R.string.latest_version,
                    String.format(Locale.US, "v%s (%d)",
                            Config.remoteManagerVersionString, Config.remoteManagerVersionCode)));
            if (Config.remoteManagerVersionCode > BuildConfig.VERSION_CODE) {
                color = colorInfo;
                image = R.drawable.ic_update;
                status = getString(R.string.manager_update_title);
                manager.install.setText(R.string.update);
            } else {
                color = colorOK;
                image = R.drawable.ic_check_circle;
                status = getString(R.string.manager_up_to_date);
                manager.install.setText(R.string.install);
            }
        }
        manager.statusIcon.setImageResource(image);
        manager.statusIcon.setColorFilter(color);
        manager.status.setText(status);

        magisk.setValid(Config.remoteMagiskVersionCode > 0);
        manager.setValid(Config.remoteManagerVersionCode > 0);

        TransitionManager.beginDelayedTransition(root, transition);

        if (Config.remoteMagiskVersionCode < 0) {
            // Hide install related components
            installOptionCard.setVisibility(View.GONE);
            uninstallButton.setVisibility(View.GONE);
        } else {
            // Show install related components
            installOptionCard.setVisibility(View.VISIBLE);
            uninstallButton.setVisibility(Shell.rootAccess() ? View.VISIBLE : View.GONE);
        }

        if (!shownDialog && !ShellUtils.fastCmdResult("env_check")) {
            shownDialog = true;
            new EnvFixDialog(requireActivity()).show();
        }
    }
}

