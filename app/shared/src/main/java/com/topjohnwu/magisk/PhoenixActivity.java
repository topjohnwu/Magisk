package com.topjohnwu.magisk;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Process;

import io.michaelrocks.paranoid.Obfuscate;

// Inspired by https://github.com/JakeWharton/ProcessPhoenix

@Obfuscate
public class PhoenixActivity extends Activity {

    private static final String PID_KEY = "pid";

    public static void rebirth(Context context, String clzName) {
        var intent = new Intent();
        intent.setComponent(new ComponentName(context, clzName));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(PID_KEY, Process.myPid());
        context.startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Process.killProcess(getIntent().getIntExtra(PID_KEY, -1));
        var intent = getPackageManager().getLaunchIntentForPackage(getPackageName());
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(intent);
        finish();
        Runtime.getRuntime().exit(0);
    }
}
