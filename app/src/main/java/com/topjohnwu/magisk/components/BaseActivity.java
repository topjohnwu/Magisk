package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Download;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public abstract class BaseActivity extends FlavorActivity {

    public static final String INTENT_PERM = "perm_dialog";

    protected static Runnable permissionGrantCallback;

    public static void runWithPermission(Context context, String[] permissions, Runnable callback) {
        boolean granted = true;
        for (String perm : permissions) {
            if (ContextCompat.checkSelfPermission(context, perm) != PackageManager.PERMISSION_GRANTED)
                granted = false;
        }
        if (granted) {
            Download.EXTERNAL_PATH.mkdirs();
            callback.run();
        } else {
            // Passed in context should be an activity if not granted, need to show dialog!
            permissionGrantCallback = callback;
            if (!(context instanceof BaseActivity)) {
                // Start activity to show dialog
                Intent intent = new Intent(context, a.g.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.putExtra(INTENT_PERM, permissions);
                context.startActivity(intent);
            } else {
                ActivityCompat.requestPermissions((BaseActivity) context, permissions, 0);
            }
        }
    }

    public void runWithPermission(String[] permissions, Runnable callback) {
        runWithPermission(this, permissions, callback);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String[] perms = getIntent().getStringArrayExtra(INTENT_PERM);
        if (perms != null)
            ActivityCompat.requestPermissions(this, perms, 0);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        boolean grant = true;
        for (int result : grantResults) {
            if (result != PackageManager.PERMISSION_GRANTED)
                grant = false;
        }
        if (grant) {
            if (permissionGrantCallback != null) {
                permissionGrantCallback.run();
            }
        } else {
            Toast.makeText(this, R.string.no_rw_storage, Toast.LENGTH_LONG).show();
        }
        permissionGrantCallback = null;
    }
}
