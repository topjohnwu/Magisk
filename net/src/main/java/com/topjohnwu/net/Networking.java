package com.topjohnwu.net;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;

import javax.net.ssl.HttpsURLConnection;

public class Networking {

    private static final int READ_TIMEOUT = 15000;
    private static final int CONNECT_TIMEOUT = 15000;
    static Handler mainHandler = new Handler(Looper.getMainLooper());

    private static Request request(String url, String method) {
        try {
            HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
            conn.setRequestMethod(method);
            conn.setReadTimeout(READ_TIMEOUT);
            conn.setConnectTimeout(CONNECT_TIMEOUT);
            return new Request(conn);
        } catch (IOException e) {
            return new BadRequest(e);
        }
    }

    public static Request get(String url) {
        return request(url, "GET");
    }

    public static void init(Context context) {
        try {
            // Try installing new SSL provider from Google Play Service
            Context gms = context.createPackageContext("com.google.android.gms",
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            gms.getClassLoader()
                    .loadClass("com.google.android.gms.common.security.ProviderInstallerImpl")
                    .getMethod("insertProvider", Context.class)
                    .invoke(null, context);
        } catch (Exception e) {
            // Failed to update SSL provider, use NoSSLv3SocketFactory on SDK < 21
            if (Build.VERSION.SDK_INT < 21)
                HttpsURLConnection.setDefaultSSLSocketFactory(new NoSSLv3SocketFactory());
        }
    }

    public static boolean checkNetworkStatus(Context context) {
        ConnectivityManager manager = (ConnectivityManager)
                context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = manager.getActiveNetworkInfo();
        return networkInfo != null && networkInfo.isConnected();
    }
}
