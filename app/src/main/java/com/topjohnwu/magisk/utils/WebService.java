package com.topjohnwu.magisk.utils;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.List;
import java.util.Map;

import javax.net.ssl.HttpsURLConnection;

public class WebService {

    public final static int GET = 1;
    public final static int POST = 2;

    /**
     * Making web service call
     *
     * @url - url to make request
     * @requestmethod - http request method
     */
    public static String request(String url, int method) {
        return request(url, method, null, true);
    }

    public static String request(String url, int method, boolean newline) {
        return request(url, method, null, newline);
    }

    /**
     * Making service call
     *
     * @url - url to make request
     * @requestmethod - http request method
     * @params - http request params
     * @header - http request header
     * @newline - true to append a newline each line
     */
    public static String request(String urlAddress, int method,
                                 Map<String, String> header, boolean newline) {
        Logger.dev("WebService: Service call " + urlAddress);
        URL url;
        StringBuilder response = new StringBuilder();
        try {
            url = new URL(urlAddress);

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

            int responseCode = conn.getResponseCode();

            if (responseCode == HttpsURLConnection.HTTP_OK) {
                String line;
                BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
                while ((line = br.readLine()) != null) {
                    if (newline) {
                        response.append(line).append("\n");
                    } else {
                        response.append(line);
                    }
                }
                if (header != null) {
                    header.clear();
                    for (Map.Entry<String, List<String>> entry : conn.getHeaderFields().entrySet()) {
                        List<String> l = entry.getValue();
                        header.put(entry.getKey(), l.get(l.size() - 1));
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return response.toString();
    }

}