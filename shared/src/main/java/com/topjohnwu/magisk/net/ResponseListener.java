package com.topjohnwu.magisk.net;

public interface ResponseListener<T> {
    void onResponse(T response);
}
