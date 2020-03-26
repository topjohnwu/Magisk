package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import com.topjohnwu.magisk.net.ErrorHandler;
import com.topjohnwu.magisk.net.Networking;
import com.topjohnwu.magisk.net.ResponseListener;
import com.topjohnwu.magisk.obfuscate.Mapping;
import com.topjohnwu.magisk.obfuscate.RawData;
import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONException;
import org.json.JSONObject;

import static android.R.string.no;
import static android.R.string.ok;
import static android.R.string.yes;
import static com.topjohnwu.magisk.DelegateApplication.MANAGER_APK;

public class DownloadActivity extends Activity {

    private static final boolean CANARY = !BuildConfig.VERSION_NAME.contains(".");
    private static final String URL =
            BuildConfig.DEV_CHANNEL != null ? BuildConfig.DEV_CHANNEL : RawData.urlBase() +
            (BuildConfig.DEBUG ? RawData.debug() : (CANARY ? RawData.canary() : RawData.stable()));

    private String apkLink;
    private ErrorHandler err = (conn, e) -> {
        Log.e(getClass().getSimpleName(), "", e);
        finish();
    };

    private void showDialog() {
        ProgressDialog.show(this,
                RawData.dling(),
                RawData.dling() + " " + RawData.appName(),
                true);
    }

    private void dlAPK() {
        showDialog();
        if (Build.VERSION.SDK_INT >= 28) {
            // Download and relaunch the app
            Networking.get(apkLink)
                    .setErrorHandler(err)
                    .getAsFile(MANAGER_APK, apk -> {
                        Intent intent = new Intent()
                                .setComponent(new ComponentName(this, Mapping.inverse("a.r")));
                        ProcessPhoenix.triggerRebirth(this, intent);
                    });
        } else {
            // Download and upgrade the app
            Application app = getApplication();
            Networking.get(apkLink)
                    .setErrorHandler(err)
                    .getAsFile(MANAGER_APK, apk -> APKInstall.install(app, apk));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        RawData.res = getResources();
        Networking.init(this);

        if (Networking.checkNetworkStatus(this)) {
            Networking.get(URL)
                    .setErrorHandler(err)
                    .getAsJSONObject(new JSONLoader());
        } else {
            new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(RawData.appName())
                    .setMessage(RawData.networkError())
                    .setNegativeButton(ok, (d, w) -> finish())
                    .show();
        }
    }

    class JSONLoader implements ResponseListener<JSONObject> {

        @Override
        public void onResponse(JSONObject json) {
            try {
                JSONObject manager = json.getJSONObject("app");
                apkLink = manager.getString("link");
                new AlertDialog.Builder(DownloadActivity.this)
                        .setCancelable(false)
                        .setTitle(RawData.appName())
                        .setMessage(RawData.upgradeMsg())
                        .setPositiveButton(yes, (d, w) -> dlAPK())
                        .setNegativeButton(no, (d, w) -> finish())
                        .show();
            } catch (JSONException e) {
                finish();
            }
        }
    }
}
