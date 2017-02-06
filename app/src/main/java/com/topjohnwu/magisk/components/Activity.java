package com.topjohnwu.magisk.components;

import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.MagiskManager;

public class Activity extends AppCompatActivity {

    public MagiskManager getTopApplication() {
        return (MagiskManager) getApplication();
    }
}
