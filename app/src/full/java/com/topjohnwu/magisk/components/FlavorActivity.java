package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.WindowManager;

import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.Topic;

import androidx.annotation.Nullable;
import androidx.annotation.StyleRes;
import androidx.appcompat.app.AppCompatActivity;

public abstract class FlavorActivity extends AppCompatActivity implements Topic.AutoSubscriber {

    private ActivityResultListener activityResultListener;
    static int[] EMPTY_INT_ARRAY = new int[0];
    public MagiskManager mm;

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        Configuration config = base.getResources().getConfiguration();
        config.setLocale(LocaleManager.locale);
        applyOverrideConfiguration(config);
        mm = Data.MM();
    }

    @Override
    public int[] getSubscribedTopics() {
        return EMPTY_INT_ARRAY;
    }

    @StyleRes
    public int getDarkTheme() {
        return -1;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        Topic.subscribe(this);
        if (Data.isDarkTheme && getDarkTheme() != -1) {
            setTheme(getDarkTheme());
        }
        super.onCreate(savedInstanceState);
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
