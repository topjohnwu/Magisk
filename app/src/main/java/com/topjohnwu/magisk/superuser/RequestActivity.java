package com.topjohnwu.magisk.superuser;

import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.components.Activity;

public class RequestActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        if (intent == null) {
            finish();
            return;
        }

        getApplicationContext().initSuConfigs();
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK).setClass(this, SuRequestActivity.class);
        startActivity(intent);
        finish();
    }
}
