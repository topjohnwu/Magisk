package com.topjohnwu.magisk.components;

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.annotation.StyleRes;
import android.support.v7.app.AppCompatActivity;
import android.view.WindowManager;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Topic;

public abstract class FlavorActivity extends AppCompatActivity {

    private ActivityResultListener activityResultListener;

    public FlavorActivity() {
        super();
        Configuration configuration = new Configuration();
        configuration.setLocale(MagiskManager.locale);
        applyOverrideConfiguration(configuration);
    }

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
