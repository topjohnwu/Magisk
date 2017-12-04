package com.topjohnwu.crypto;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class ByteArrayStream extends ByteArrayOutputStream {
    public byte[] getBuf() {
        return buf;
    }
    public synchronized void readFrom(InputStream is) {
        readFrom(is, Integer.MAX_VALUE);
    }
    public synchronized void readFrom(InputStream is, int len) {
        int read;
        byte buffer[] = new byte[4096];
        try {
            while ((read = is.read(buffer, 0, len < buffer.length ? len : buffer.length)) > 0) {
                write(buffer, 0, read);
                len -= read;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public synchronized void writeTo(OutputStream out, int off, int len) throws IOException {
        out.write(buf, off, len);
    }
    public ByteArrayInputStream getInputStream() {
        return new ByteArrayInputStream(buf, 0, count);
    }
}
