package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.os.Bundle;

import com.topjohnwu.magisk.utils.APKInstall;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.ResponseListener;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;

public class MainActivity extends Activity {

    private static final String URL =
            "https://raw.githubusercontent.com/topjohnwu/magisk_files/master/" +
            (BuildConfig.VERSION_NAME.contains("-") ? "canary_builds/release.json" : "stable.json");

    private String apkLink;

    private void dlAPK() {
        Application app = getApplication();
        Networking.get(apkLink)
                .getAsFile(new File(getFilesDir(), "manager.apk"), apk -> APKInstall.install(app, apk));
        finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Networking.init(this);
        if (Networking.checkNetworkStatus(this)) {
            Networking.get(URL)
                    .setErrorHandler(((conn, e) -> finish()))
                    .getAsJSONObject(new JSONLoader());
        } else {
            new AlertDialog.Builder(this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
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
                new AlertDialog.Builder(MainActivity.this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
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
