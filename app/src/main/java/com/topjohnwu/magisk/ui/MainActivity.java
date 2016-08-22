package com.topjohnwu.magisk.ui;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.topjohnwu.magisk.R;

import java.io.File;

import static com.topjohnwu.magisk.ui.utils.Utils.executeCommand;

public class MainActivity extends Activity {

    private Switch rootToggle, selinuxToggle;
    private TextView magiskVersion, rootStatus, selinuxStatus, safetyNet, permissive;
    private String suPath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        magiskVersion = (TextView) findViewById(R.id.magisk_version);
        rootToggle = (Switch) findViewById(R.id.root_toggle);
        selinuxToggle = (Switch) findViewById(R.id.selinux_toggle);
        rootStatus = (TextView) findViewById(R.id.root_status);
        selinuxStatus = (TextView) findViewById(R.id.selinux_status);
        safetyNet = (TextView) findViewById(R.id.safety_net);
        permissive = (TextView) findViewById(R.id.permissive);

        suPath = executeCommand("getprop magisk.supath");
        updateStatus();

        rootToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                executeCommand(b ? "setprop magisk.root 1" : "setprop magisk.root 0");
                updateStatus();
            }
        });

        selinuxToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                executeCommand(b ? "setenforce 1" : "setenforce 0");
                updateStatus();
            }
        });

        //findViewById(R.id.modules).setOnClickListener(view -> startActivity(new Intent(this, ModulesActivity.class)));
    }

    private void updateStatus() {
        String selinux = executeCommand("getenforce");

        magiskVersion.setText(getString(R.string.magisk_version, executeCommand("getprop magisk.version")));
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

            if (!new File(suPath + "/su").exists()) {
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

            if (!new File(suPath + "/su").exists()) {
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
