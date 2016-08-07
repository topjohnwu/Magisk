package com.topjohnwu.magisk;

import android.app.Activity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class MainActivity extends Activity {

    private Switch selinuxSwitch;
    private TextView rootStatus, selinuxStatus, safetyNet, permissive;
    private Button rootButton;
    private EditText countdown;

    private String execute(String command) {

        StringBuffer output = new StringBuffer();

        Process p;
        try {
            p = Runtime.getRuntime().exec(command);
            p.waitFor();
            BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()));

            String line = "";
            while ((line = reader.readLine())!= null) {
                output.append(line + "\n");
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        String response = output.toString();
        return response;

    }

    private void updateStatus() {
        String selinux = execute("getenforce");

        if((new File("/system/xbin/su").exists())) {
            rootStatus.setText("Mounted");
            rootStatus.setTextColor(Color.RED);
            safetyNet.setText("Root mounted and enabled. Safety Net (Android Pay) will NOT work");
            safetyNet.setTextColor(Color.RED);
            rootButton.setEnabled(true);
        } else {
            rootStatus.setText("Not Mounted");
            rootStatus.setTextColor(Color.GREEN);
            safetyNet.setText("Safety Net (Android Pay) should work, but no root temporarily");
            safetyNet.setTextColor(Color.GREEN);
            rootButton.setEnabled(false);
        }

        selinuxStatus.setText(selinux);

        if(selinux.equals("Enforcing\n")) {
            selinuxStatus.setTextColor(Color.GREEN);
            selinuxSwitch.setChecked(true);
            permissive.setText("SELinux is enforced");
            permissive.setTextColor(Color.GREEN);
        } else {
            selinuxStatus.setTextColor(Color.RED);
            selinuxSwitch.setChecked(false);
            permissive.setText("Only turn off SELinux if necessary!");
            permissive.setTextColor(Color.RED);
        }
    }

    protected class SU extends AsyncTask<String, Void, Void> {

        @Override
        protected Void doInBackground(String... params) {
            try {
                Process su = Runtime.getRuntime().exec("su");
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

        if(!(new File("/magisk/phh/su")).exists()) {
            setContentView(R.layout.no_root);
        } else {

            setContentView(R.layout.activity_main);

            selinuxSwitch = (Switch) findViewById(R.id.permissive_switch);
            rootStatus = (TextView) findViewById(R.id.root_status);
            selinuxStatus = (TextView) findViewById(R.id.selinux_status);
            safetyNet = (TextView) findViewById(R.id.safety_net);
            permissive = (TextView) findViewById(R.id.permissive);
            countdown = (EditText) findViewById(R.id.countdown);
            rootButton = (Button) findViewById(R.id.rootButton);

            updateStatus();

            rootButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    int timeout;
                    timeout = Integer.parseInt(countdown.getText().toString()) * 60;
                    (new SU()).execute("setprop magisk.timeout " + String.valueOf(timeout), "setprop magisk.phhsu 0");
                }
            });

            selinuxSwitch.setOnClickListener(new View.OnClickListener() {
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
}
