package com.topjohnwu.magisk.components;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.WindowManager;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StyleRes;
import androidx.appcompat.app.AppCompatActivity;
import androidx.collection.SparseArrayCompat;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Event;
import com.topjohnwu.magisk.utils.LocaleManager;

public abstract class BaseActivity extends AppCompatActivity implements Event.AutoListener {

    private static Runnable grantCallback;

    static int[] EMPTY_INT_ARRAY = new int[0];

    private SparseArrayCompat<ActivityResultListener> resultListeners = new SparseArrayCompat<>();
    public App app = App.self;

    @Override
    public int[] getListeningEvents() {
        return EMPTY_INT_ARRAY;
    }

    @Override
    public void onEvent(int event) {}

    @StyleRes
    public int getDarkTheme() {
        return -1;
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(LocaleManager.getLocaleContext(base, LocaleManager.locale));
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        Event.register(this);
        if (getDarkTheme() != -1 && (boolean) Config.get(Config.Key.DARK_THEME)) {
            setTheme(getDarkTheme());
        }
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onDestroy() {
        Event.unregister(this);
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

    protected void lockOrientation() {
        if (Build.VERSION.SDK_INT < 18)
            setRequestedOrientation(getResources().getConfiguration().orientation);
        else
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LOCKED);
    }

    public void runWithExternalRW(Runnable callback) {
        runWithPermission(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, callback);
    }

    public void runWithPermission(String[] permissions, Runnable callback) {
        runWithPermission(this, permissions, callback);
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
            if (context instanceof BaseActivity) {
                grantCallback = callback;
                ActivityCompat.requestPermissions((BaseActivity) context, permissions, 0);
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        ActivityResultListener listener = resultListeners.get(requestCode);
        if (listener != null) {
            resultListeners.remove(requestCode);
            listener.onActivityResult(resultCode, data);
        }
    }

    public void startActivityForResult(Intent intent, int requestCode, ActivityResultListener listener) {
        resultListeners.put(requestCode, listener);
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
            if (grantCallback != null) {
                grantCallback.run();
            }
        } else {
            Toast.makeText(this, R.string.no_rw_storage, Toast.LENGTH_LONG).show();
        }
        grantCallback = null;
    }

    public interface ActivityResultListener {
        void onActivityResult(int resultCode, Intent data);
    }

    @Override
    public SharedPreferences getSharedPreferences(String name, int mode) {
        if (TextUtils.equals(name, getPackageName() + "_preferences"))
            return app.prefs;
        return super.getSharedPreferences(name, mode);
    }
}
