package com.topjohnwu.magisk;

import android.support.annotation.NonNull;

import com.topjohnwu.magisk.components.Activity;

public class NoUIActivity extends Activity {
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        finish();
    }
}
