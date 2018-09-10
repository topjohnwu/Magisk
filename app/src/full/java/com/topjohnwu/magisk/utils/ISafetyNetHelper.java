package com.topjohnwu.magisk.utils;

import androidx.annotation.Keep;

public interface ISafetyNetHelper {

    int RESPONSE_ERR = 0x01;
    int CONNECTION_FAIL = 0x02;

    int BASIC_PASS = 0x10;
    int CTS_PASS = 0x20;

    @Keep
    void attest();

    @Keep
    int getVersion();

    interface Callback {
        @Keep
        void onResponse(int responseCode);
    }
}
