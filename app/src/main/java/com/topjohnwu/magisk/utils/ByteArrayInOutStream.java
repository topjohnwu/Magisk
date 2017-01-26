package com.topjohnwu.magisk.utils;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

public class ByteArrayInOutStream extends ByteArrayOutputStream {
    public ByteArrayInputStream getInputStream() {
        ByteArrayInputStream in = new ByteArrayInputStream(buf, 0, count);
        count = 0;
        buf = new byte[32];
        return in;
    }

    public void setBuffer(byte[] buffer) {
        buf = buffer;
        count = buffer.length;
    }
}
