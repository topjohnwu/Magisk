package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextThemeWrapper;

import com.topjohnwu.magisk.net.ErrorHandler;
import com.topjohnwu.magisk.net.Networking;
import com.topjohnwu.magisk.net.ResponseListener;
import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;

import static android.R.string.no;
import static android.R.string.ok;
import static android.R.string.yes;

public class MainActivity extends Activity {

    private static final boolean CANARY = !BuildConfig.VERSION_NAME.contains(".");
    private static final String URL =
            BuildConfig.DEV_CHANNEL != null ? BuildConfig.DEV_CHANNEL :
            "https://raw.githubusercontent.com/topjohnwu/magisk_files/" +
            (BuildConfig.DEBUG ? "canary/debug.json" :
            (CANARY ? "canary/release.json" : "master/stable.json"));
    private static final String APP_NAME = "Magisk Manager";

    private String apkLink;
    private ErrorHandler err = (conn, e) -> {
        Log.e(getClass().getSimpleName(), "", e);
        finish();
    };
    private Context themed;

    private void showDialog() {
        ProgressDialog.show(themed,
            getString(R.string.dling),
            getString(R.string.dling) + " " + APP_NAME,
            true);
    }

    private void dlAPK() {
        showDialog();
        // Download and upgrade the app
        Application app = getApplication();
        Networking.get(apkLink)
                .setErrorHandler(err)
                .getAsFile(
                        new File(getCacheDir(), "manager.apk"),
                        apk -> APKInstall.install(app, apk)
                );
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Networking.init(this);
        themed = new ContextThemeWrapper(this, android.R.style.Theme_DeviceDefault);

        if (Networking.checkNetworkStatus(this)) {
            Networking.get(URL)
                    .setErrorHandler(err)
                    .getAsJSONObject(new JSONLoader());
        } else {
            new AlertDialog.Builder(themed)
                    .setCancelable(false)
                    .setTitle(APP_NAME)
                    .setMessage(getString(R.string.no_internet_msg))
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
                new AlertDialog.Builder(themed)
                        .setCancelable(false)
                        .setTitle(APP_NAME)
                        .setMessage(getString(R.string.upgrade_msg))
                        .setPositiveButton(yes, (d, w) -> dlAPK())
                        .setNegativeButton(no, (d, w) -> finish())
                        .show();
            } catch (JSONException e) {
                finish();
            }
        }
    }
}
