package com.topjohnwu.magisk.superuser;

import android.content.Intent;
import android.os.Bundle;

import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.SuRequestActivity;
import com.topjohnwu.magisk.components.BaseActivity;

public class RequestActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = new Intent(this, Data.classMap.get(SuRequestActivity.class))
                .putExtra("socket", getIntent().getStringExtra("socket"))
                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
        finish();
    }
}
