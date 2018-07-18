package com.topjohnwu.magisk.utils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Map;

public class WebService {

    public static String getString(String url) {
        return getString(url, null);
    }

    public static String getString(String url, Map<String, String> header) {
        try {
            HttpURLConnection conn = request(url, header);
            return getString(conn);
        } catch (IOException e) {
            e.printStackTrace();
            return "";
        }
    }

    public static String getString(HttpURLConnection conn) {
        try {
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
            return builder.toString();
        } catch (IOException e) {
            e.printStackTrace();
            return "";
        }
    }

    public static HttpURLConnection request(String address, Map<String, String> header) throws IOException {
        URL url = new URL(address);

        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setReadTimeout(15000);
        conn.setConnectTimeout(15000);

        if (header != null) {
            for (Map.Entry<String, String> entry : header.entrySet()) {
                conn.setRequestProperty(entry.getKey(), entry.getValue());
            }
        }

        conn.connect();

        if (header != null) {
            header.clear();
            for (Map.Entry<String, List<String>> entry : conn.getHeaderFields().entrySet()) {
                List<String> l = entry.getValue();
                header.put(entry.getKey(), l.get(l.size() - 1));
            }
        }

        return conn;
    }
}
