package com.topjohnwu.unhide;

import android.app.Activity;
import android.os.Bundle;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Locale;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String command = String.format(
                "pm unhide com.topjohnwu.magisk\n" +
                "am start -n com.topjohnwu.magisk/.SplashActivity\n" +
                "pm uninstall %s\n" +
                "exit\n",
                getApplicationInfo().packageName);
        Process process;
        try {
            process = Runtime.getRuntime().exec("su");
            OutputStream in = process.getOutputStream();
            in.write(command.getBytes("UTF-8"));
            in.flush();
            process.waitFor();
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
        finish();
    }
}
