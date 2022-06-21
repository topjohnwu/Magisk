package com.topjohnwu.magisk;

import static android.R.string.no;
import static android.R.string.ok;
import static android.R.string.yes;
import static com.topjohnwu.magisk.R.string.dling;
import static com.topjohnwu.magisk.R.string.no_internet_msg;
import static com.topjohnwu.magisk.R.string.upgrade_msg;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.res.loader.ResourcesLoader;
import android.content.res.loader.ResourcesProvider;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.system.ErrnoException;
import android.system.Os;
import android.system.OsConstants;
import android.util.Log;
import android.view.ContextThemeWrapper;

import com.topjohnwu.magisk.net.Networking;
import com.topjohnwu.magisk.net.Request;
import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONException;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.zip.CRC32;
import java.util.zip.InflaterInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

import io.michaelrocks.paranoid.Obfuscate;

@Obfuscate
public class DownloadActivity extends Activity {

    private static final String APP_NAME = "Magisk";
    private static final String JSON_URL = BuildConfig.DEBUG ?
            "https://topjohnwu.github.io/magisk-files/debug.json" :
            "https://topjohnwu.github.io/magisk-files/canary.json";

    private String apkLink = BuildConfig.APK_URL;
    private Context themed;
    private ProgressDialog dialog;
    private boolean dynLoad;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (DynLoad.activeClassLoader instanceof AppClassLoader) {
            // For some reason activity is created before Application.attach(),
            // relaunch the activity using the same intent
            finishAffinity();
            startActivity(getIntent());
            return;
        }

        themed = new ContextThemeWrapper(this, android.R.style.Theme_DeviceDefault);

        // Only download and dynamic load full APK if hidden
        dynLoad = !getPackageName().equals(BuildConfig.APPLICATION_ID);

        // Inject resources
        try {
            loadResources();
        } catch (Exception e) {
            Log.e(APP_NAME, Log.getStackTraceString(e));
        }

        ProviderInstaller.install(this);

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

    @Override
    public void finish() {
        super.finish();
        Runtime.getRuntime().exit(0);
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
        request(JSON_URL).getAsJSONObject(json -> {
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
        var request = request(apkLink).setExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        if (dynLoad) {
            request.getAsFile(StubApk.current(this), file -> StubApk.restartProcess(this));
        } else {
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

    private void decryptResources(OutputStream out) throws IOException {
        try (var zip = new ZipOutputStream(out)) {
            var buf = new ByteArrayOutputStream();
            var arsc = new InflaterInputStream(new ByteArrayInputStream(Bytes.res()));
            try (arsc) {
                APKInstall.transfer(arsc, buf);
            }
            var data = buf.toByteArray();
            var crc = new CRC32();
            crc.update(data);
            var arscEntry = new ZipEntry("resources.arsc");
            arscEntry.setMethod(ZipEntry.STORED);
            arscEntry.setSize(data.length);
            arscEntry.setCrc(crc.getValue());
            zip.putNextEntry(arscEntry);
            zip.write(data);
            zip.closeEntry();

            zip.putNextEntry(new ZipEntry("AndroidManifest.xml"));
            var apk = new ZipFile(getPackageResourcePath());
            var xml = apk.getInputStream(apk.getEntry("AndroidManifest.xml"));
            try (apk; xml) {
                APKInstall.transfer(xml, zip);
            }
            zip.closeEntry();

            zip.finish();
        }
    }

    private void loadResources() throws IOException, ErrnoException {
        if (Build.VERSION.SDK_INT >= 30) {
            var fd = Os.memfd_create("res.apk", 0);
            try {
                decryptResources(new FileOutputStream(fd));
                Os.lseek(fd, 0, OsConstants.SEEK_SET);
                try (var pfd = ParcelFileDescriptor.dup(fd)) {
                    var loader = new ResourcesLoader();
                    loader.addProvider(ResourcesProvider.loadFromApk(pfd));
                    getResources().addLoaders(loader);
                }
            } finally {
                Os.close(fd);
            }
        } else {
            File apk = new File(getCacheDir(), "res.apk");
            decryptResources(new FileOutputStream(apk));
            StubApk.addAssetPath(getResources(), apk.getPath());
        }
    }
}
