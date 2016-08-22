package com.topjohnwu.magisk;

import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;

public class RootFragment extends Fragment {

    private Switch rootToggle, selinuxToggle;
    private TextView magiskVersion, rootStatus, selinuxStatus, safetyNet, permissive;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.root_fragment, container, false);

        magiskVersion = (TextView) view.findViewById(R.id.magisk_version);
        rootToggle = (Switch) view.findViewById(R.id.root_toggle);
        selinuxToggle = (Switch) view.findViewById(R.id.selinux_toggle);
        rootStatus = (TextView) view.findViewById(R.id.root_status);
        selinuxStatus = (TextView) view.findViewById(R.id.selinux_status);
        safetyNet = (TextView) view.findViewById(R.id.safety_net);
        permissive = (TextView) view.findViewById(R.id.permissive);

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

        return view;
    }

    private void updateStatus() {
        String selinux = Utils.sh("getenforce");
        String version = Utils.sh("getprop magisk.version");

        magiskVersion.setText(getString(R.string.magisk_version, version));
        selinuxStatus.setText(selinux);

        if (selinux.equals("Enforcing")) {
            selinuxStatus.setTextColor(Color.GREEN);
            selinuxToggle.setChecked(true);
            permissive.setText(R.string.selinux_enforcing_info);
            permissive.setTextColor(Color.GREEN);
        } else {
            selinuxStatus.setTextColor(Color.RED);
            selinuxToggle.setChecked(false);
            permissive.setText(R.string.selinux_permissive_info);
            permissive.setTextColor(Color.RED);
        }

        if (new File("/system/framework/twframework.jar").exists()) {
            selinuxToggle.setEnabled(false);
            permissive.setText(R.string.selinux_samsung);
        }

        if (new File("/system/xbin/su").exists()) {
            rootStatus.setTextColor(Color.RED);
            safetyNet.setTextColor(Color.RED);
            rootToggle.setChecked(true);

            if (!Utils.rootAccess) {
                rootStatus.setText(R.string.root_system);
                safetyNet.setText(R.string.root_system_info);
                rootToggle.setEnabled(false);
                selinuxToggle.setEnabled(false);
            } else {
                rootStatus.setText(R.string.root_mounted);
                safetyNet.setText(R.string.root_mounted_info);
            }
        } else {
            rootStatus.setTextColor(Color.GREEN);
            safetyNet.setTextColor(Color.GREEN);
            rootToggle.setChecked(false);

            if (!Utils.rootAccess) {
                rootStatus.setText(R.string.root_none);
                safetyNet.setText(R.string.root_none_info);
                rootToggle.setEnabled(false);
                selinuxToggle.setEnabled(false);
            } else {
                rootStatus.setText(R.string.root_unmounted);
                safetyNet.setText(R.string.root_unmounted_info);
            }
        }
    }
}
