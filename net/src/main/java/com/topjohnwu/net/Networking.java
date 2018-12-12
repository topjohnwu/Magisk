package com.topjohnwu.net;

import android.os.Handler;
import android.os.Looper;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.ExecutorService;

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
            return new StubRequest();
        }
    }

    public static Request get(String url) {
        return request(url, "GET");
    }

}
