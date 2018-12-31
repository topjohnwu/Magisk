package com.topjohnwu.net;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

public class ProgressInputStream extends FilterInputStream {

    private long totalBytes;
    private long bytesDownloaded;

    public ProgressInputStream(InputStream in, long total) {
        super(in);
        totalBytes = total;
    }

    protected void updateProgress(long bytesDownloaded, long totalBytes) {}

    private void update() {
        Networking.mainHandler.post(() -> updateProgress(bytesDownloaded, totalBytes));
    }

    @Override
    public int read() throws IOException {
        int b = super.read();
        if (b >= 0) {
            bytesDownloaded++;
            update();
        }
        return b;
    }

    @Override
    public int read(byte[] b) throws IOException {
        return read(b, 0, b.length);
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        int sz = super.read(b, off, len);
        if (sz > 0) {
            bytesDownloaded += sz;
            update();
        }
        return sz;
    }
}
