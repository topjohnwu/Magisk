package com.topjohnwu.magisk;

import android.support.annotation.NonNull;

import com.topjohnwu.magisk.components.BaseActivity;

public class NoUIActivity extends BaseActivity {
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        finish();
    }
}
