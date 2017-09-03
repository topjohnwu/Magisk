package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.components.Activity;

public class SplashActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getMagiskManager().startup();

        Intent intent = new Intent(this, MainActivity.class);
        String section = getIntent().getStringExtra(MagiskManager.INTENT_SECTION);
        if (section != null) {
            intent.putExtra(MagiskManager.INTENT_SECTION, section);
        }
        startActivity(intent);
        finish();
    }
}
