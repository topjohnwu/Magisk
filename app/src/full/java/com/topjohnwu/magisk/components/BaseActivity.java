package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.NoUIActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.Topic;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StyleRes;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public abstract class BaseActivity extends AppCompatActivity implements Topic.AutoSubscriber {

    public static final String INTENT_PERM = "perm_dialog";

    protected static Runnable permissionGrantCallback;
    static int[] EMPTY_INT_ARRAY = new int[0];

    private ActivityResultListener activityResultListener;
    public MagiskManager mm;

    @Override
    public int[] getSubscribedTopics() {
        return EMPTY_INT_ARRAY;
    }

    @StyleRes
    public int getDarkTheme() {
        return -1;
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        Configuration config = base.getResources().getConfiguration();
        config.setLocale(LocaleManager.locale);
        applyOverrideConfiguration(config);
        mm = Data.MM();
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        Topic.subscribe(this);
        if (Data.isDarkTheme && getDarkTheme() != -1) {
            setTheme(getDarkTheme());
        }
        super.onCreate(savedInstanceState);
        String[] perms = getIntent().getStringArrayExtra(INTENT_PERM);
        if (perms != null)
            ActivityCompat.requestPermissions(this, perms, 0);
    }

    @Override
    protected void onDestroy() {
        Topic.unsubscribe(this);
        super.onDestroy();
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

    public static void runWithPermission(Context context, String[] permissions, Runnable callback) {
        boolean granted = true;
        for (String perm : permissions) {
            if (ContextCompat.checkSelfPermission(context, perm) != PackageManager.PERMISSION_GRANTED)
                granted = false;
        }
        if (granted) {
            Const.EXTERNAL_PATH.mkdirs();
            callback.run();
        } else {
            // Passed in context should be an activity if not granted, need to show dialog!
            permissionGrantCallback = callback;
            if (!(context instanceof BaseActivity)) {
                // Start NoUIActivity to show dialog
                Intent intent = new Intent(context, Data.classMap.get(NoUIActivity.class));
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
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (activityResultListener != null)
            activityResultListener.onActivityResult(requestCode, resultCode, data);
        activityResultListener = null;
    }

    public void startActivityForResult(Intent intent, int requestCode, ActivityResultListener listener) {
        activityResultListener = listener;
        super.startActivityForResult(intent, requestCode);
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

    public interface ActivityResultListener {
        void onActivityResult(int requestCode, int resultCode, Intent data);
    }
}
