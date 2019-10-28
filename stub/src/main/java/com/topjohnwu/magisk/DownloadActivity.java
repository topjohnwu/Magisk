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
import com.topjohnwu.magisk.obfuscate.Dump;
import com.topjohnwu.magisk.obfuscate.RawData;
import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;

import static android.R.string.no;
import static android.R.string.ok;
import static android.R.string.yes;
import static com.topjohnwu.magisk.DelegateApplication.MANAGER_APK;
import static com.topjohnwu.magisk.obfuscate.R.string.dling;
import static com.topjohnwu.magisk.obfuscate.R.string.no_internet_msg;
import static com.topjohnwu.magisk.obfuscate.R.string.upgrade_msg;

public class DownloadActivity extends Activity {

    private static final String URL =
            BuildConfig.DEV_CHANNEL != null ? BuildConfig.DEV_CHANNEL :
            RawData.urlBase() + (BuildConfig.DEBUG ? RawData.canary() : RawData.stable());

    private String apkLink;
    private ErrorHandler err = (conn, e) -> {
        Log.e(getClass().getSimpleName(), "", e);
        finish();
    };

    private void showDialog() {
        ProgressDialog.show(this,
                getString(dling),
                getString(dling) + " " + RawData.appName(),
                true);
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

        // Inject resources
        File resAPK = new File(getCacheDir(), "res.apk");
        Dump.resAPK(getResources(), resAPK);
        DynAPK.addAssetPath(getResources().getAssets(), resAPK.getPath());

        Networking.init(this);
        if (Networking.checkNetworkStatus(this)) {
            Networking.get(URL)
                    .setErrorHandler(err)
                    .getAsJSONObject(new JSONLoader());
        } else {
            new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(RawData.appName())
                    .setMessage(no_internet_msg)
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
                        .setMessage(upgrade_msg)
                        .setPositiveButton(yes, (d, w) -> dlAPK())
                        .setNegativeButton(no, (d, w) -> finish())
                        .show();
            } catch (JSONException e) {
                finish();
            }
        }
    }
}
