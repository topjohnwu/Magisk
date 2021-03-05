package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.widget.Toast;

import com.topjohnwu.magisk.net.Networking;
import com.topjohnwu.magisk.net.Request;
import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONException;

import java.io.File;

import static android.R.string.no;
import static android.R.string.ok;
import static android.R.string.yes;
import static com.topjohnwu.magisk.DelegateApplication.dynLoad;
import static com.topjohnwu.magisk.R.string.dling;
import static com.topjohnwu.magisk.R.string.no_internet_msg;
import static com.topjohnwu.magisk.R.string.relaunch_app;
import static com.topjohnwu.magisk.R.string.upgrade_msg;

public class DownloadActivity extends Activity {

    private static final String APP_NAME = "Magisk";
    private static final String CANARY_URL = "https://topjohnwu.github.io/magisk-files/canary.json";

    private String apkLink = BuildConfig.APK_URL;
    private Context themed;
    private ProgressDialog dialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        themed = new ContextThemeWrapper(this, android.R.style.Theme_DeviceDefault);

        if (Networking.checkNetworkStatus(this)) {
            if (apkLink == null) {
                fetchCanary();
            } else {
                showDialog();
            }
        } else {
            new AlertDialog.Builder(themed)
                    .setCancelable(false)
                    .setTitle(APP_NAME)
                    .setMessage(getString(no_internet_msg))
                    .setNegativeButton(ok, (d, w) -> finish())
                    .show();
        }
    }

    private void error(Throwable e) {
        Log.e(getClass().getSimpleName(), "", e);
        finish();
    }

    private Request request(String url) {
        return Networking.get(url).setErrorHandler((conn, e) -> error(e));
    }

    private void showDialog() {
        new AlertDialog.Builder(themed)
                .setCancelable(false)
                .setTitle(APP_NAME)
                .setMessage(getString(upgrade_msg))
                .setPositiveButton(yes, (d, w) -> dlAPK())
                .setNegativeButton(no, (d, w) -> finish())
                .show();
    }

    private void fetchCanary() {
        dialog = ProgressDialog.show(themed, "", "", true);
        request(CANARY_URL).getAsJSONObject(json -> {
            dialog.dismiss();
            try {
                apkLink = json.getJSONObject("magisk").getString("link");
                showDialog();
            } catch (JSONException e) {
                error(e);
            }
        });
    }

    private void dlAPK() {
        dialog = ProgressDialog.show(themed, getString(dling), getString(dling) + " " + APP_NAME, true);
        // Download and upgrade the app
        File apk = dynLoad ? DynAPK.current(this) : new File(getCacheDir(), "manager.apk");
        request(apkLink).setExecutor(AsyncTask.THREAD_POOL_EXECUTOR).getAsFile(apk, file -> {
            if (dynLoad) {
                InjectAPK.setup(this);
                runOnUiThread(() -> {
                    dialog.dismiss();
                    Toast.makeText(themed, relaunch_app, Toast.LENGTH_LONG).show();
                    finish();
                });
            } else {
                runOnUiThread(() -> {
                    dialog.dismiss();
                    APKInstall.install(this, file);
                    finish();
                });
            }
        });
    }

}
