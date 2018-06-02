package com.topjohnwu.magisk.components;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.support.annotation.NonNull;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;

public abstract class Activity extends FlavorActivity {

    private ActivityResultListener activityResultListener;

    public Activity() {
        super();
        Configuration configuration = new Configuration();
        configuration.setLocale(MagiskManager.locale);
        applyOverrideConfiguration(configuration);
    }

    @Override
    public Application getApplicationContext() {
        return (Application) super.getApplicationContext();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        boolean grant = true;
        for (int result : grantResults) {
            if (result != PackageManager.PERMISSION_GRANTED)
                grant = false;
        }
        Application app = getApplicationContext();
        if (grant) {
            if (app.permissionGrantCallback != null) {
                app.permissionGrantCallback.run();
            }
        } else {
            MagiskManager.toast(R.string.no_rw_storage, Toast.LENGTH_LONG);
        }
        app.permissionGrantCallback = null;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (activityResultListener != null)
            activityResultListener.onActivityResult(requestCode, resultCode, data);
        activityResultListener = null;
    }

    public void startActivityForResult(Intent intent, int requestCode, ActivityResultListener listener) {
        activityResultListener = listener;
        super.startActivityForResult(intent, requestCode);
    }

    public interface ActivityResultListener {
        void onActivityResult(int requestCode, int resultCode, Intent data);
    }

}
