package com.topjohnwu.magisk.components;

import android.content.res.AssetManager;
import android.content.res.Resources;
import android.os.Bundle;
import android.support.annotation.Keep;
import android.support.annotation.Nullable;
import android.support.annotation.StyleRes;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Topic;

public abstract class FlavorActivity extends AppCompatActivity {

    private AssetManager swappedAssetManager = null;
    private Resources swappedResources = null;
    private Resources.Theme backupTheme = null;

    @StyleRes
    public int getDarkTheme() {
        return -1;
    }

    public MagiskManager getMagiskManager() {
        return (MagiskManager) super.getApplication();
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (this instanceof Topic.Subscriber) {
            ((Topic.Subscriber) this).subscribeTopics();
        }

        if (getMagiskManager().isDarkTheme && getDarkTheme() != -1) {
            setTheme(getDarkTheme());
        }
    }

    @Override
    protected void onDestroy() {
        if (this instanceof Topic.Subscriber) {
            ((Topic.Subscriber) this).unsubscribeTopics();
        }
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

    @Override
    public Resources.Theme getTheme() {
        return backupTheme == null ? super.getTheme() : backupTheme;
    }

    @Override
    public AssetManager getAssets() {
        return swappedAssetManager == null ? super.getAssets() : swappedAssetManager;
    }

    private AssetManager getAssets(String apk) {
        try {
            AssetManager asset = AssetManager.class.newInstance();
            AssetManager.class.getMethod("addAssetPath", String.class).invoke(asset, apk);
            return asset;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    @Override
    public Resources getResources() {
        return swappedResources == null ? super.getResources() : swappedResources;
    }

    @Keep
    public void swapResources(String dexPath) {
        AssetManager asset = getAssets(dexPath);
        if (asset != null) {
            backupTheme = super.getTheme();
            Resources res = super.getResources();
            swappedResources = new Resources(asset, res.getDisplayMetrics(), res.getConfiguration());
            swappedAssetManager = asset;
        }
    }

    @Keep
    public void restoreResources() {
        swappedAssetManager = null;
        swappedResources = null;
        backupTheme = null;
    }
}
