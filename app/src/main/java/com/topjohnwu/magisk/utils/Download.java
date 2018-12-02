package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;

import java.io.File;

public class Download {

    public static final File EXTERNAL_PATH =
            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

    static {
        EXTERNAL_PATH.mkdirs();
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
