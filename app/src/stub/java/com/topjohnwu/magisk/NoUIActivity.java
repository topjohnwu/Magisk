package com.topjohnwu.magisk;

import android.Manifest;
import android.app.AlertDialog;
import android.os.AsyncTask;
import android.os.Bundle;

import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.receivers.ManagerInstall;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Locale;

public class NoUIActivity extends BaseActivity {

    private String apkLink;
    private String version;
    private int versionCode;

    public static final String URL =
            "https://raw.githubusercontent.com/topjohnwu/magisk_files/master/stable.json";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Download.checkNetworkStatus(this)) {
            AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                String str = WebService.getString(URL);
                try {
                    JSONObject json = new JSONObject(str);
                    JSONObject manager = json.getJSONObject("app");
                    version = manager.getString("version");
                    versionCode = manager.getInt("versionCode");
                    apkLink = manager.getString("link");
                } catch (JSONException e) {
                    e.printStackTrace();
                    finish();
                    return;
                }
                runOnUiThread(() -> {
                    String filename = String.format(Locale.US, "MagiskManager-v%s(%d).apk",
                            version, versionCode);
                    new AlertDialog.Builder(this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
                            .setCancelable(false)
                            .setTitle(R.string.app_name)
                            .setMessage(R.string.upgrade_msg)
                            .setPositiveButton(R.string.yes, (d, w) -> runWithPermission(new String[]
                                    { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
                                Download.receive(this, new ManagerInstall(), apkLink, filename);
                                finish();
                            }))
                            .setNegativeButton(R.string.no_thanks, (d, w) -> finish())
                            .show();
                });
            });
        } else {
            new AlertDialog.Builder(this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
                    .setCancelable(false)
                    .setTitle(R.string.app_name)
                    .setMessage(R.string.no_internet_msg)
                    .setNegativeButton(R.string.ok, (d, w) -> finish())
                    .show();
        }

    }
}
