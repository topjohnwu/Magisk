package com.topjohnwu.magisk.utils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Map;

import javax.net.ssl.HttpsURLConnection;

public class WebService {

    public final static int GET = 1;
    public final static int POST = 2;

    public static String getString(String url) {
        return getString(url, null);
    }

    public static String getString(String url, Map<String, String> header) {
        InputStream in  = request(GET, url, header);
        if (in == null) return "";
        BufferedReader br = new BufferedReader(new InputStreamReader(in));
        int len;
        StringBuilder builder = new StringBuilder();
        char buf[] = new char[4096];
        try {
            while ((len = br.read(buf)) != -1) {
                builder.append(buf, 0, len);
            }
            in.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return builder.toString();
    }

    public static InputStream request(int method, String address, Map<String, String> header) {
        Logger.dev("WebService: Service call " + address);
        try {
            URL url = new URL(address);

            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(15000);
            conn.setConnectTimeout(15000);
            conn.setDoInput(true);

            if (method == POST) {
                conn.setRequestMethod("POST");
            } else if (method == GET) {
                conn.setRequestMethod("GET");
            }

            if (header != null) {
                for (Map.Entry<String, String> entry : header.entrySet()) {
                    conn.setRequestProperty(entry.getKey(), entry.getValue());
                }
            }

            if (conn.getResponseCode() == HttpsURLConnection.HTTP_OK) {
                if (header != null) {
                    header.clear();
                    for (Map.Entry<String, List<String>> entry : conn.getHeaderFields().entrySet()) {
                        List<String> l = entry.getValue();
                        header.put(entry.getKey(), l.get(l.size() - 1));
                    }
                }
                return conn.getInputStream();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

}