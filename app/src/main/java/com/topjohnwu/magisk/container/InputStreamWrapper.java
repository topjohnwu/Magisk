package com.topjohnwu.magisk.container;

import android.support.annotation.NonNull;

import java.io.IOException;
import java.io.InputStream;

public class InputStreamWrapper extends InputStream {
    private InputStream in;

    public InputStreamWrapper(InputStream in) {
        this.in = in;
    }

    @Override
    public int available() throws IOException {
        return in.available();
    }

    @Override
    public void close() throws IOException {
        in.close();
    }

    @Override
    public synchronized void mark(int readlimit) {
        in.mark(readlimit);
    }

    @Override
    public boolean markSupported() {
        return in.markSupported();
    }

    @Override
    public synchronized int read() throws IOException {
        return in.read();
    }

    @Override
    public int read(@NonNull byte[] b) throws IOException {
        return in.read(b);
    }

    @Override
    public synchronized int read(@NonNull byte[] b, int off, int len) throws IOException {
        return in.read(b, off, len);
    }

    @Override
    public synchronized void reset() throws IOException {
        in.reset();
    }

    @Override
    public long skip(long n) throws IOException {
        return in.skip(n);
    }

    @Override
    public int hashCode() {
        return in.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        return in.equals(obj);
    }

    @Override
    public String toString() {
        return in.toString();
    }
}
