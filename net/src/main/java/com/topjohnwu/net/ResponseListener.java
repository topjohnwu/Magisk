package com.topjohnwu.net;

public interface ResponseListener<T> {
    void onResponse(T response);
}
