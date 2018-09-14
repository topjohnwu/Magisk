package com.topjohnwu.magisk;

import com.topjohnwu.magisk.components.BaseActivity;

import androidx.annotation.NonNull;

public class NoUIActivity extends BaseActivity {
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        finish();
    }
}
