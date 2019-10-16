package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.ProgressDialog;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import com.topjohnwu.magisk.net.ErrorHandler;
import com.topjohnwu.magisk.net.Networking;
import com.topjohnwu.magisk.net.ResponseListener;
import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONException;
import org.json.JSONObject;

import static com.topjohnwu.magisk.DelegateApplication.MANAGER_APK;

public class DownloadActivity extends Activity {

    static final String TAG = "MMStub";
    private static final boolean IS_CANARY = BuildConfig.VERSION_NAME.contains("-");
    private static final String URL = BuildConfig.DEV_CHANNEL != null ? BuildConfig.DEV_CHANNEL :
            "https://raw.githubusercontent.com/topjohnwu/magisk_files/" +
                    (IS_CANARY ? "canary/release.json" : "master/stable.json");

    private String apkLink;
    private ErrorHandler err = (conn, e) -> {
        Log.e(TAG, "network error", e);
        finish();
    };

    private void showDialog() {
        ProgressDialog.show(this,
                "Downloading...",
                "Downloading Magisk Manager", true);
    }

    private void dlAPK() {
        showDialog();
        if (Build.VERSION.SDK_INT >= 28) {
            // Download and relaunch the app
            Networking.get(apkLink)
                    .setErrorHandler(err)
                    .getAsFile(MANAGER_APK, f -> ProcessPhoenix.triggerRebirth(this));
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
        Networking.init(this);
        if (Networking.checkNetworkStatus(this)) {
            Networking.get(URL)
                    .setErrorHandler(err)
                    .getAsJSONObject(new JSONLoader());
        } else {
            new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(R.string.app_name)
                    .setMessage(R.string.no_internet_msg)
                    .setNegativeButton(R.string.ok, (d, w) -> finish())
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
                        .setTitle(R.string.app_name)
                        .setMessage(R.string.upgrade_msg)
                        .setPositiveButton(R.string.yes, (d, w) -> dlAPK())
                        .setNegativeButton(R.string.no_thanks, (d, w) -> finish())
                        .show();
            } catch (JSONException e) {
                finish();
            }
        }
    }
}
