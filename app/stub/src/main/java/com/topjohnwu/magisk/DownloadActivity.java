package com.topjohnwu.magisk;

import static android.R.string.no;
import static android.R.string.ok;
import static android.R.string.yes;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;

import com.topjohnwu.magisk.net.Networking;
import com.topjohnwu.magisk.net.Request;
import com.topjohnwu.magisk.utils.APKInstall;

import java.io.IOException;

public class DownloadActivity extends Activity {

    private static final String APP_NAME = "Magisk";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getTheme().applyStyle(android.R.style.Theme_DeviceDefault_Dialog_NoActionBar, true);

        ProviderInstaller.install(this);

        if (Networking.checkNetworkStatus(this)) {
            showDialog();
        } else {
            new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(APP_NAME)
                    .setMessage(R.string.no_internet_msg)
                    .setNegativeButton(ok, (d, w) -> finish())
                    .show();
        }
    }

    @Override
    public void finish() {
        super.finish();
        Runtime.getRuntime().exit(0);
    }

    private void error(Throwable e) {
        Log.e(getClass().getSimpleName(), Log.getStackTraceString(e));
        finish();
    }

    private Request request(String url) {
        return Networking.get(url).setErrorHandler((conn, e) -> error(e));
    }

    private void showDialog() {
        new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(APP_NAME)
                .setMessage(R.string.upgrade_msg)
                .setPositiveButton(yes, (d, w) -> dlAPK())
                .setNegativeButton(no, (d, w) -> finish())
                .show();
    }

    private void dlAPK() {
        ProgressDialog.show(this, getString(R.string.dling), getString(R.string.dling) + " " + APP_NAME, true);
        var request = request(BuildConfig.APK_URL).setExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        request.getAsInputStream(input -> {
            var session = APKInstall.startSession(this);
            try (input; var out = session.openStream(this)) {
                if (out != null)
                    APKInstall.transfer(input, out);
            } catch (IOException e) {
                error(e);
            }
            Intent intent = session.waitIntent();
            if (intent != null)
                startActivity(intent);
        });
    }
}
