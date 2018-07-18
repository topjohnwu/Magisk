package com.topjohnwu.magisk.utils;

import android.support.annotation.Keep;

public interface ISafetyNetHelper {

    int CAUSE_SERVICE_DISCONNECTED = 0x01;
    int CAUSE_NETWORK_LOST = 0x02;
    int RESPONSE_ERR = 0x04;
    int CONNECTION_FAIL = 0x08;

    int BASIC_PASS = 0x10;
    int CTS_PASS = 0x20;

    void attest();
    int getVersion();

    interface Callback {
        @Keep
        void onResponse(int responseCode);
    }
}
