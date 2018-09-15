package com.topjohnwu.magisk.utils;

public interface ISafetyNetHelper {

    int RESPONSE_ERR = 0x01;
    int CONNECTION_FAIL = 0x02;

    int BASIC_PASS = 0x10;
    int CTS_PASS = 0x20;

    void attest();

    int getVersion();

    interface Callback {
        void onResponse(int responseCode);
    }
}
