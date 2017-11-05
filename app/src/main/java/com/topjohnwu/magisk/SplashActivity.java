package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.Const;

public class SplashActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getMagiskManager().startup();

        Intent intent = new Intent(this, MainActivity.class);
        String section = getIntent().getStringExtra(Const.Key.OPEN_SECTION);
        if (section != null) {
            intent.putExtra(Const.Key.OPEN_SECTION, section);
        }
        startActivity(intent);
        finish();
    }
}
