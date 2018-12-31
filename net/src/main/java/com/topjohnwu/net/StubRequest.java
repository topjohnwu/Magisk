package com.topjohnwu.net;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.io.InputStream;
import java.util.concurrent.Executor;

class StubRequest extends Request {

    StubRequest() { super(null); }

    @Override
    public Request addHeaders(String key, String value) { return this; }

    @Override
    public Request setDownloadProgressListener(DownloadProgressListener listener) { return this; }

    @Override
    public Request setErrorHandler(ErrorHandler handler) { return this; }

    @Override
    public Request setExecutor(Executor e) { return this; }

    @Override
    public Result<InputStream> execForInputStream() { return new Result<>(); }

    @Override
    public void getAsFile(ResponseListener<File> rs, File out) {}

    @Override
    public void getAsString(ResponseListener<String> rs) {}

    @Override
    public Result<String> execForString() { return new Result<>(); }

    @Override
    public void getAsJSONObject(ResponseListener<JSONObject> rs){}

    @Override
    public Result<JSONObject> execForJSONObject() { return new Result<>(); }

    @Override
    public void getAsJSONArray(ResponseListener<JSONArray> rs){}

    @Override
    public Result<JSONArray> execForJSONArray() { return new Result<>(); }
}
