package com.topjohnwu.magisk;

import com.topjohnwu.magisk.components.Application;

public class MagiskManager extends Application {

    public int magiskVersionCode = -1;

    public static MagiskManager get() {
        return (MagiskManager) weakSelf.get();
    }

    public void dumpPrefs() {/* NOP */}
}
