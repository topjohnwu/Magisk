package com.topjohnwu.magisk.components;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Bundle;
import android.support.annotation.Keep;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

public class Activity extends AppCompatActivity {

    private AssetManager swappedAssetManager = null;
    private Resources swappedResources = null;
    private Resources.Theme swappedTheme = null;
    private ActivityResultListener activityResultListener;

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
        MagiskManager mm = getMagiskManager();
        if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            if (mm.permissionGrantCallback != null) {
                mm.permissionGrantCallback.run();
            }
        }
        mm.permissionGrantCallback = null;
    }

    @Override
    public Resources.Theme getTheme() {
        return swappedTheme == null ? super.getTheme() : swappedTheme;
    }

    @Override
    public AssetManager getAssets() {
        return swappedAssetManager == null ? super.getAssets() : swappedAssetManager;
    }

    @Override
    public Resources getResources() {
        return swappedResources == null ? super.getResources() : swappedResources;
    }

    public MagiskManager getMagiskManager() {
        return (MagiskManager) super.getApplicationContext();
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

    @Keep
    public void swapResources(String dexPath, int resId) {
        swappedAssetManager = Utils.getAssets(dexPath);
        if (swappedAssetManager == null)
            return;
        Resources res = super.getResources();
        swappedResources = new Resources(swappedAssetManager, res.getDisplayMetrics(), res.getConfiguration());
        swappedTheme = swappedResources.newTheme();
        swappedTheme.applyStyle(resId, true);
    }

    @Keep
    public void restoreResources() {
        swappedAssetManager = null;
        swappedResources = null;
        swappedTheme = null;
    }

    public interface ActivityResultListener {
        void onActivityResult(int requestCode, int resultCode, Intent data);
    }

}
