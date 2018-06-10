package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.widget.Toast;

import com.topjohnwu.magisk.NoUIActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Const;

public abstract class Activity extends FlavorActivity {

    protected static Runnable permissionGrantCallback;

    private ActivityResultListener activityResultListener;

    public Activity() {
        super();
        Configuration configuration = new Configuration();
        configuration.setLocale(Application.locale);
        applyOverrideConfiguration(configuration);
    }

    public static void runWithPermission(Context context, String[] permissions, Runnable callback) {
        boolean granted = true;
        for (String perm : permissions) {
            if (ContextCompat.checkSelfPermission(context, perm) != PackageManager.PERMISSION_GRANTED)
                granted = false;
        }
        if (granted) {
            callback.run();
        } else {
            // Passed in context should be an activity if not granted, need to show dialog!
            permissionGrantCallback = callback;
            if (!(context instanceof Activity)) {
                // Start activity to show dialog
                Intent intent = new Intent(context, NoUIActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.putExtra(Const.Key.INTENT_PERM, permissions);
                context.startActivity(intent);
            } else {
                ActivityCompat.requestPermissions((Activity) context, permissions, 0);
            }
        }
    }

    public void runWithPermission(String[] permissions, Runnable callback) {
        runWithPermission(this, permissions, callback);
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
        if (grant) {
            if (permissionGrantCallback != null) {
                permissionGrantCallback.run();
            }
        } else {
            Application.toast(R.string.no_rw_storage, Toast.LENGTH_LONG);
        }
        permissionGrantCallback = null;
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
