package com.topjohnwu.magisk;

import android.app.Activity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Switch;
import android.widget.TextView;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class MainActivity extends Activity {

    private Switch rootToggle, selinuxToggle;
    private TextView magiskVersion, rootStatus, selinuxStatus, safetyNet, permissive;
    private String suPath;

    private String execute(String command) {

        StringBuilder output = new StringBuilder();

        Process p;
        try {
            p = Runtime.getRuntime().exec(command);
            p.waitFor();
            BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()));

            String line;
            while ((line = reader.readLine())!= null) {
                if(output.length() != 0) output.append("\n");
                output.append(line);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        return output.toString();

    }

    private void updateStatus() {

        String selinux = execute("getenforce");
        magiskVersion.setText(getString(R.string.magisk_version, execute("getprop magisk.version")));
        selinuxStatus.setText(selinux);

        if(selinux.equals("Enforcing")) {
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

        if((new File("/system/framework/twframework.jar")).exists()) {
            selinuxToggle.setEnabled(false);
            permissive.setText(R.string.selinux_samsung);
        }

        if((new File("/system/xbin/su").exists())) {
            rootStatus.setTextColor(Color.RED);
            safetyNet.setTextColor(Color.RED);
            rootToggle.setChecked(true);

            if(!(new File(suPath + "/su")).exists()) {
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

            if(!(new File(suPath + "/su")).exists()) {
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

    protected class SU extends AsyncTask<String, Void, Void> {

        @Override
        protected Void doInBackground(String... params) {
            try {
                Process su = Runtime.getRuntime().exec(suPath + "/su");
                DataOutputStream out = new DataOutputStream(su.getOutputStream());
                for(String command : params) {
                    out.writeBytes(command + "\n");
                    out.flush();
                }
                out.writeBytes("exit\n");
                out.flush();
            } catch (IOException e) { e.printStackTrace(); }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            final Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    updateStatus();
                }
            }, 1500);
        }
    }

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

        suPath = execute("getprop magisk.supath");

        setContentView(R.layout.activity_main);

        magiskVersion = (TextView) findViewById(R.id.magisk_version);
        rootToggle = (Switch) findViewById(R.id.root_toggle);
        selinuxToggle = (Switch) findViewById(R.id.selinux_toggle);
        rootStatus = (TextView) findViewById(R.id.root_status);
        selinuxStatus = (TextView) findViewById(R.id.selinux_status);
        safetyNet = (TextView) findViewById(R.id.safety_net);
        permissive = (TextView) findViewById(R.id.permissive);

        updateStatus();

        rootToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch s = (Switch) view;
                if(s.isChecked()) {
                    (new SU()).execute("setprop magisk.root 1");
                } else {
                    (new SU()).execute("setprop magisk.root 0");
                }
            }
        });

        selinuxToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch s = (Switch) view;
                if(s.isChecked()) {
                    (new SU()).execute("setenforce 1");
                } else {
                    (new SU()).execute("setenforce 0");
                }
            }
        });
    }
}
