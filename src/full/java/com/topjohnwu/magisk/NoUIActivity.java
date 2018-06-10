package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;

import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.utils.Const;

public class NoUIActivity extends Activity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String[] perms = getIntent().getStringArrayExtra(Const.Key.INTENT_PERM);
        if (perms != null) {
            ActivityCompat.requestPermissions(this, perms, 0);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        finish();
    }
}
