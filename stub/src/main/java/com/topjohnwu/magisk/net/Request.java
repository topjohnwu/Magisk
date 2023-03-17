package com.topjohnwu.magisk.net;

import android.os.AsyncTask;

import com.topjohnwu.magisk.utils.APKInstall;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.util.Scanner;
import java.util.concurrent.Executor;

public class Request {
    private final HttpURLConnection conn;
    private Executor executor = null;
    private int code = -1;

    ErrorHandler err = null;

    private interface Requestor<T> {
        T request() throws Exception;
    }

    public class Result<T> {
        T result;

        public T getResult() {
            return result;
        }

        public int getCode() {
            return code;
        }

        public boolean isSuccess() {
            return code >= 200 && code <= 299;
        }

        public HttpURLConnection getConnection() {
            return conn;
        }
    }

    Request(HttpURLConnection c) {
        conn = c;
    }

    public Request addHeaders(String key, String value) {
        conn.setRequestProperty(key, value);
        return this;
    }

    public Request setErrorHandler(ErrorHandler handler) {
        err = handler;
        return this;
    }

    public Request setExecutor(Executor e) {
        executor = e;
        return this;
    }

    public Result<Void> connect() {
        try {
            connect0();
        } catch (IOException e) {
            if (err != null)
                err.onError(conn, e);
        }
        return new Result<>();
    }

    public Result<InputStream> execForInputStream() {
        return exec(this::getInputStream);
    }

    public void getAsInputStream(ResponseListener<InputStream> rs) {
        submit(this::getInputStream, rs);
    }

    public void getAsFile(File out, ResponseListener<File> rs) {
        submit(() -> dlFile(out), rs);
    }

    public void execForFile(File out) {
        exec(() -> dlFile(out));
    }

    public void getAsBytes(ResponseListener<byte[]> rs) {
        submit(this::dlBytes, rs);
    }

    public Result<byte[]> execForBytes() {
        return exec(this::dlBytes);
    }

    public void getAsString(ResponseListener<String> rs) {
        submit(this::dlString, rs);
    }

    public Result<String> execForString() {
        return exec(this::dlString);
    }

    public void getAsJSONObject(ResponseListener<JSONObject> rs) {
        submit(this::dlJSONObject, rs);
    }

    public Result<JSONObject> execForJSONObject() {
        return exec(this::dlJSONObject);
    }

    public void getAsJSONArray(ResponseListener<JSONArray> rs) {
        submit(this::dlJSONArray, rs);
    }

    public Result<JSONArray> execForJSONArray() {
        return exec(this::dlJSONArray);
    }

    private void connect0() throws IOException {
        conn.connect();
        code = conn.getResponseCode();
    }

    private <T> Result<T> exec(Requestor<T> req) {
        Result<T> res = new Result<>();
        try {
            res.result = req.request();
        } catch (Exception e) {
            if (err != null)
                err.onError(conn, e);
        }
        return res;
    }

    private <T> void submit(Requestor<T> req, ResponseListener<T> rs) {
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            try {
                T t = req.request();
                Runnable cb = () -> rs.onResponse(t);
                if (executor == null)
                    Networking.mainHandler.post(cb);
                else
                    executor.execute(cb);
            } catch (Exception e) {
                if (err != null)
                    err.onError(conn, e);
            }
        });
    }

    private BufferedInputStream getInputStream() throws IOException {
        connect0();
        InputStream in = new FilterInputStream(conn.getInputStream()) {
            @Override
            public void close() throws IOException {
                super.close();
                conn.disconnect();
            }
        };
        return new BufferedInputStream(in);
    }

    private String dlString() throws IOException {
        try (Scanner s = new Scanner(getInputStream(), "UTF-8")) {
            s.useDelimiter("\\A");
            return s.next();
        }
    }

    private JSONObject dlJSONObject() throws IOException, JSONException {
        return new JSONObject(dlString());
    }

    private JSONArray dlJSONArray() throws IOException, JSONException {
        return new JSONArray(dlString());
    }

    private File dlFile(File f) throws IOException {
        try (InputStream in = getInputStream();
             OutputStream out = new BufferedOutputStream(new FileOutputStream(f))) {
            APKInstall.transfer(in, out);
        }
        return f;
    }

    private byte[] dlBytes() throws IOException {
        int len = conn.getContentLength();
        len = len > 0 ? len : 32;
        ByteArrayOutputStream out = new ByteArrayOutputStream(len);
        try (InputStream in = getInputStream()) {
            APKInstall.transfer(in, out);
        }
        return out.toByteArray();
    }
}
