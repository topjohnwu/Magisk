package com.topjohnwu.net;

import android.os.AsyncTask;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Reader;
import java.net.HttpURLConnection;
import java.util.concurrent.Executor;

public class Request {
    private HttpURLConnection conn;
    private Executor executor = null;
    private DownloadProgressListener progress = null;
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

    public Request setDownloadProgressListener(DownloadProgressListener listener) {
        progress = listener;
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

    public Result<InputStream> execForInputStream() {
        return exec(this::getInputStream);
    }

    public void getAsFile(File out, ResponseListener<File> rs) {
        submit(() -> dlFile(out), rs);
    }

    public void execForFile(File out) {
        exec(() -> dlFile(out));
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
        conn.connect();
        code = conn.getResponseCode();
        InputStream in = conn.getInputStream();
        if (progress != null) {
            in = new ProgressInputStream(in, conn.getContentLength(), progress) {
                @Override
                public void close() throws IOException {
                    super.close();
                    conn.disconnect();
                }
            };
        } else {
            in = new FilterInputStream(in) {
                @Override
                public void close() throws IOException {
                    super.close();
                    conn.disconnect();
                }
            };
        }
        return new BufferedInputStream(in);
    }

    private String dlString() throws IOException {
        StringBuilder builder = new StringBuilder();
        try (Reader reader = new InputStreamReader(getInputStream())) {
            int len;
            char buf[] = new char[4096];
            while ((len = reader.read(buf)) != -1) {
                builder.append(buf, 0, len);
            }
        }
        return builder.toString();
    }

    private JSONObject dlJSONObject() throws IOException, JSONException {
        return new JSONObject(dlString());
    }

    private JSONArray dlJSONArray() throws IOException, JSONException {
        return new JSONArray(dlString());
    }

    private File dlFile(File f) throws IOException {
        try (InputStream in  = getInputStream();
             OutputStream out = new BufferedOutputStream(new FileOutputStream(f))) {
            int len;
            byte buf[] = new byte[4096];
            while ((len = in.read(buf)) != -1) {
                out.write(buf, 0, len);
            }
        }
        return f;
    }
}
