package com.topjohnwu.magisk.utils;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;

public class WebService {

    public static HttpURLConnection request(String address) throws IOException {
        URL url = new URL(address);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setReadTimeout(15000);
        conn.setConnectTimeout(15000);
        conn.connect();
        return conn;
    }

    public static HttpURLConnection mustRequest(String address) throws IOException {
        HttpURLConnection conn;
        do {
            conn = WebService.request(address);
            int total = conn.getContentLength();
            if (total < 0)
                conn.disconnect();
            else
                break;
        } while (true);
        return conn;
    }
}
