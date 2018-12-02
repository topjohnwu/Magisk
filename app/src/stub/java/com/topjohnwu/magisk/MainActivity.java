package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.os.AsyncTask;
import android.os.Bundle;

import com.topjohnwu.magisk.utils.APKInstall;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;

public class MainActivity extends Activity {

    private static final String URL =
            "https://raw.githubusercontent.com/topjohnwu/magisk_files/master/stable.json";

    private String apkLink;

    private void dlAPK() {
        Application app = getApplication();
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            try {
                HttpURLConnection conn = WebService.request(apkLink);
                File apk = new File(getFilesDir(), "manager.apk");
                try (InputStream in = new BufferedInputStream(conn.getInputStream());
                     OutputStream out = new BufferedOutputStream(new FileOutputStream(apk))) {
                    int len;
                    byte[] buf = new byte[4096];
                    while ((len = in.read(buf)) != -1) {
                        out.write(buf, 0, len);
                    }
                }
                conn.disconnect();
                APKInstall.install(app, apk);
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        finish();
    }

    private void dlJSON() throws IOException, JSONException {
        HttpURLConnection conn = WebService.request(URL);
        StringBuilder builder = new StringBuilder();
        if (conn.getResponseCode() == HttpURLConnection.HTTP_OK) {
            try (BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()))) {
                int len;
                char buf[] = new char[4096];
                while ((len = br.read(buf)) != -1) {
                    builder.append(buf, 0, len);
                }
            }
        }
        conn.disconnect();
        JSONObject json = new JSONObject(builder.toString());
        JSONObject manager = json.getJSONObject("app");
        apkLink = manager.getString("link");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Download.checkNetworkStatus(this)) {
            AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                try {
                    dlJSON();
                    runOnUiThread(() -> {
                        new AlertDialog.Builder(this, AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
                                .setCancelable(false)
                                .setTitle(R.string.app_name)
                                .setMessage(R.string.upgrade_msg)
                                .setPositiveButton(R.string.yes, (d, w) -> dlAPK())
                                .setNegativeButton(R.string.no_thanks, (d, w) -> finish())
                                .show();
                    });
                } catch (JSONException | IOException e) {
                    finish();
                }
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
