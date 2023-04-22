package com.topjohnwu.magisk.dummy;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class DummyService extends Service {
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
