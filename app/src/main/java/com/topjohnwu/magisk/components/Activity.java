package com.topjohnwu.magisk.components;

import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;

public class Activity extends AppCompatActivity {

    private Runnable permissionGrantCallback;

    public Activity() {
        super();
        Configuration configuration = new Configuration();
        configuration.setLocale(MagiskManager.locale);
        applyOverrideConfiguration(configuration);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (this instanceof Topic.Subscriber) {
            ((Topic.Subscriber) this).subscribeTopics();
        }
    }

    @Override
    protected void onDestroy() {
        if (this instanceof Topic.Subscriber) {
            ((Topic.Subscriber) this).unsubscribeTopics();
        }
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            if (permissionGrantCallback != null) {
                permissionGrantCallback.run();
            }
        }
        permissionGrantCallback = null;
    }

    public void setPermissionGrantCallback(Runnable callback) {
        permissionGrantCallback = callback;
    }

    public MagiskManager getMagiskManager() {
        return (MagiskManager) super.getApplicationContext();
    }

    public Shell getShell() {
        return Shell.getShell(this);
    }

    protected void setFloating() {
        boolean isTablet = getResources().getBoolean(R.bool.isTablet);
        if (isTablet) {
            WindowManager.LayoutParams params = getWindow().getAttributes();
            params.height = getResources().getDimensionPixelSize(R.dimen.floating_height);
            params.width = getResources().getDimensionPixelSize(R.dimen.floating_width);
            params.alpha = 1.0f;
            params.dimAmount = 0.6f;
            params.flags |= 2;
            getWindow().setAttributes(params);
            setFinishOnTouchOutside(true);
        }
    }

}
