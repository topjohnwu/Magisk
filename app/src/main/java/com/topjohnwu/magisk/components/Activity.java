package com.topjohnwu.magisk.components;

import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.MagiskManager;

public class Activity extends AppCompatActivity {

    @Override
    public MagiskManager getApplicationContext() {
        return (MagiskManager) getApplication();
    }
}
