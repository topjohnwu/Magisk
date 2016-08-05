package com.topjohnwu.magisk;

import android.app.Activity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Switch;
import android.widget.TextView;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;

public class MainActivity extends Activity {

    private String suPATH;
    private String xbinPATH;

    private Switch rootSwitch, selinuxSwitch;
    private TextView rootStatus, selinuxStatus, safetyNet, permissive;

    protected class callSU extends AsyncTask<String, Void, String[]> {

        @Override
        protected String[] doInBackground(String... params) {
            String[] results = new String[2];
            try {
                Process su = Runtime.getRuntime().exec(suPATH);
                DataOutputStream out = new DataOutputStream(su.getOutputStream());
                DataInputStream in = new DataInputStream(su.getInputStream());
                for(int i = 0; i < params.length; ++i) {
                    out.writeBytes(params[i] + "\n");
                    out.flush();
                }
                out.writeBytes("if [ -z $(which su) ]; then echo 0; else echo 1; fi;\n");
                out.flush();
                results[0] = in.readLine();
                out.writeBytes("getenforce\n");
                out.flush();
                results[1] = in.readLine();
                out.writeBytes("exit\n");
                out.flush();
            } catch (IOException e) { e.printStackTrace(); }
            return results;
        }
        @Override
        protected void onPostExecute(String[] results) {
            if(results[0].equals("1")) {
                rootStatus.setText("Mounted");
                rootStatus.setTextColor(Color.RED);
                rootSwitch.setChecked(true);
                safetyNet.setText("Root mounted and enabled. Safety Net (Android Pay) will NOT work");
                safetyNet.setTextColor(Color.RED);
            } else {
                rootStatus.setText("Not Mounted");
                rootStatus.setTextColor(Color.GREEN);
                rootSwitch.setChecked(false);
                safetyNet.setText("Safety Net (Android Pay) should work, but no root temporarily");
                safetyNet.setTextColor(Color.GREEN);
            }

            selinuxStatus.setText(results[1]);

            if(results[1].equals("Enforcing")) {
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
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        boolean rooted = true;

        File phh = new File("/magisk/phh/su");
        File supersu = new File("/su/bin/su");

        if(!supersu.exists()) {
            if(!phh.exists()) {
                setContentView(R.layout.no_root);
                rooted = false;
            } else {
                suPATH = "/magisk/phh/su";
                xbinPATH = "/magisk/phh/xbin";
            }
        } else {
            suPATH = "/su/bin/su";
            xbinPATH = "/su/xbin";
        }

        if(rooted) {

            setContentView(R.layout.activity_main);

            rootSwitch = (Switch) findViewById(R.id.root_switch);
            selinuxSwitch = (Switch) findViewById(R.id.permissive_switch);
            rootStatus = (TextView) findViewById(R.id.root_status);
            selinuxStatus = (TextView) findViewById(R.id.selinux_status);
            safetyNet = (TextView) findViewById(R.id.safety_net);
            permissive = (TextView) findViewById(R.id.permissive);

            (new callSU()).execute();

            rootSwitch.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Switch s = (Switch) view;
                    if(s.isChecked()) {
                        (new callSU()).execute("mount -o bind " + xbinPATH + " /system/xbin");
                    } else {
                        (new callSU()).execute("umount /system/xbin");
                    }
                }
            });

            selinuxSwitch.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Switch s = (Switch) view;
                    if(s.isChecked()) {
                        (new callSU()).execute("setenforce 1");
                    } else {
                        (new callSU()).execute("setenforce 0");
                    }
                }
            });
        }
    }
}
