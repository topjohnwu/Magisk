package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Switch;
import android.widget.TextView;

import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

import butterknife.BindColor;
import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskFragment extends Fragment {

    @BindView(R.id.rootSwitchView) View rootToggleView;
    @BindView(R.id.root_toggle) Switch rootToggle;

    @BindView(R.id.selinuxSwitchView) View selinuxToggleView;
    @BindView(R.id.selinux_toggle) Switch selinuxToggle;

    @BindView(R.id.magisk_status_container) View magiskStatusContainer;
    @BindView(R.id.magisk_status_icon) ImageView magiskStatusIcon;
    @BindView(R.id.magisk_version) TextView magiskVersion;

    @BindView(R.id.root_status_container) View rootStatusContainer;
    @BindView(R.id.root_status_icon) ImageView rootStatusIcon;
    @BindView(R.id.root_status) TextView rootStatus;

    @BindView(R.id.selinux_status_container) View selinuxStatusContainer;
    @BindView(R.id.selinux_status_icon) ImageView selinuxStatusIcon;
    @BindView(R.id.selinux_status) TextView selinuxStatus;

    @BindView(R.id.safety_net_status) TextView safetyNetStatus;
    @BindView(R.id.safety_net_icon) ImageView safetyNetStatusIcon;

    @BindColor(R.color.red500) int red500;
    @BindColor(R.color.green500) int green500;
    @BindColor(R.color.grey500) int grey500;

    int statusOK = R.drawable.ic_check_circle;
    int statusError = R.drawable.ic_error;
    int statusUnknown = R.drawable.ic_help;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.magisk_fragment, container, false);
        ButterKnife.bind(this, view);

        updateStatus();

        rootToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Utils.su(b ? "setprop magisk.root 1" : "setprop magisk.root 0");
                updateStatus();
            }
        });

        selinuxToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                Utils.su(b ? "setenforce 1" : "setenforce 0");
                updateStatus();
            }
        });

        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                updateStatus(); // update status again
            }
        }, 1000);


        return view;
    }

    private void updateStatus() {
        String selinux = Utils.sh("getenforce");
        String version = Utils.sh("getprop magisk.version");

        if (TextUtils.isEmpty(version)) {
            magiskStatusContainer.setBackgroundColor(grey500);
            magiskStatusIcon.setImageResource(statusUnknown);

            magiskVersion.setTextColor(grey500);
            magiskVersion.setText("?");
        } else {
            magiskStatusContainer.setBackgroundColor(green500);
            magiskStatusIcon.setImageResource(statusOK);

            magiskVersion.setTextColor(green500);
            magiskVersion.setText(getString(R.string.magisk_version, version));
        }

        if (selinux.equals("Enforcing")) {
            selinuxStatusContainer.setBackgroundColor(green500);
            selinuxStatusIcon.setImageResource(statusOK);

            selinuxStatus.setText(R.string.selinux_enforcing_info);
            selinuxStatus.setTextColor(green500);
            selinuxToggle.setChecked(true);
        } else {
            selinuxStatusContainer.setBackgroundColor(red500);
            selinuxStatusIcon.setImageResource(statusError);

            selinuxStatus.setText(R.string.selinux_permissive_info);
            selinuxStatus.setTextColor(red500);
            selinuxToggle.setChecked(false);
        }

        if (new File("/system/framework/twframework.jar").exists()) {
            selinuxToggleView.setVisibility(View.GONE);
            selinuxStatus.append("\n" + getString(R.string.selinux_samsung));
        }

        if (new File("/system/xbin/su").exists()) {
            rootStatusContainer.setBackgroundColor(red500);
            rootStatusIcon.setImageResource(statusError);

            rootStatus.setTextColor(red500);
            rootToggle.setChecked(true);

            safetyNetStatusIcon.setImageResource(statusError);

            if (!Utils.rootAccess) {
                rootStatusContainer.setBackgroundColor(red500);
                rootStatusIcon.setImageResource(statusError);
                rootStatus.setTextColor(red500);
                rootStatus.setText(R.string.root_system);

                rootToggleView.setVisibility(View.GONE);
                selinuxToggleView.setVisibility(View.GONE);

                safetyNetStatus.setText(R.string.root_system_info);
            } else {
                rootStatusContainer.setBackgroundColor(green500);
                rootStatusIcon.setImageResource(statusOK);
                rootStatus.setTextColor(green500);
                rootStatus.setText(R.string.root_mounted);

                safetyNetStatus.setText(R.string.root_mounted_info);
            }
        } else {
            rootStatusContainer.setBackgroundColor(green500);
            rootStatusIcon.setImageResource(red500);

            rootStatus.setTextColor(green500);
            rootToggle.setChecked(false);

            safetyNetStatusIcon.setImageResource(statusOK);

            if (!Utils.rootAccess) {
                rootStatusContainer.setBackgroundColor(red500);
                rootStatusIcon.setImageResource(statusError);
                rootStatus.setTextColor(red500);
                rootStatus.setText(R.string.root_none);

                rootToggleView.setVisibility(View.GONE);
                selinuxToggleView.setVisibility(View.GONE);

                safetyNetStatusIcon.setImageResource(statusError);
                safetyNetStatus.setText(R.string.root_none_info);
            } else {
                rootStatus.setText(R.string.root_unmounted);
                safetyNetStatus.setText(R.string.root_unmounted_info);
            }
        }
    }
}
