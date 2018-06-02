package com.topjohnwu.magisk;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;

import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.receivers.ManagerUpdate;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class SplashActivity extends Activity {

    private String apkLink;
    private String version;
    private int versionCode;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Utils.checkNetworkStatus()) {
            AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                String str = WebService.getString(Const.Url.STABLE_URL);
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
                    String filename = Utils.fmt("MagiskManager-v%s(%d).apk", version, versionCode);
                    new AlertDialog.Builder(this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
                            .setCancelable(false)
                            .setTitle(R.string.app_name)
                            .setMessage(R.string.upgrade_msg)
                            .setPositiveButton(R.string.yes, (d, w) -> {
                                Utils.runWithPermission(this, new String[]
                                        { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
                                    Intent intent = new Intent(this, ManagerUpdate.class);
                                    intent.putExtra(Const.Key.INTENT_SET_LINK, apkLink);
                                    intent.putExtra(Const.Key.INTENT_SET_FILENAME, filename);
                                    sendBroadcast(intent);
                                    finish();
                                });
                            })
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
