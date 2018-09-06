package com.topjohnwu.magisk.utils;

import android.Manifest;
import android.app.DownloadManager;
import android.content.Context;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Environment;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.receivers.DownloadReceiver;

import java.io.File;

public class Download {

    public static final File EXTERNAL_PATH =
            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

    static {
        EXTERNAL_PATH.mkdirs();
    }

    public static boolean isDownloading = false;

    public static void receive(Context context, DownloadReceiver receiver, String link, String filename) {
        if (isDownloading)
            return;

        BaseActivity.runWithPermission(context,
                new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
            File file = new File(EXTERNAL_PATH, getLegalFilename(filename));
            file.delete();

            Toast.makeText(context, context.getString(R.string.downloading_toast, filename),
                    Toast.LENGTH_LONG).show();

            isDownloading = true;

            DownloadManager.Request request = new DownloadManager
                    .Request(Uri.parse(link))
                    .setDestinationUri(Uri.fromFile(file));

            DownloadManager dm = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
            receiver.setDownloadID(dm.enqueue(request)).setFile(file);
            context.getApplicationContext().registerReceiver(receiver,
                    new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
        });
    }

    public static String getLegalFilename(CharSequence filename) {
        return filename.toString().replace(" ", "_").replace("'", "").replace("\"", "")
                .replace("$", "").replace("`", "").replace("*", "").replace("/", "_")
                .replace("#", "").replace("@", "").replace("\\", "_");
    }

    public static boolean checkNetworkStatus(Context context) {
        ConnectivityManager manager = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = manager.getActiveNetworkInfo();
        return networkInfo != null && networkInfo.isConnected();
    }
}
