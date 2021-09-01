package com.topjohnwu.magisk.signing;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class ByteArrayStream extends ByteArrayOutputStream {

    public synchronized void readFrom(InputStream is) {
        readFrom(is, Integer.MAX_VALUE);
    }

    public synchronized void readFrom(InputStream is, int len) {
        int read;
        byte buffer[] = new byte[4096];
        try {
            while ((read = is.read(buffer, 0, Math.min(len, buffer.length))) > 0) {
                write(buffer, 0, read);
                len -= read;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public ByteArrayInputStream getInputStream() {
        return new ByteArrayInputStream(buf, 0, count);
    }
}
