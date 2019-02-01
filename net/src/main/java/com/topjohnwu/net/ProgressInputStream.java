package com.topjohnwu.net;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

public class ProgressInputStream extends FilterInputStream {

    private long totalBytes;
    private long bytesDownloaded;
    private DownloadProgressListener progress;

    public ProgressInputStream(InputStream in, long total, DownloadProgressListener listener) {
        super(in);
        totalBytes = total;
        progress = listener;
    }

    @Override
    public int read() throws IOException {
        int b = super.read();
        if (totalBytes > 0 && b >= 0) {
            bytesDownloaded++;
            Networking.mainHandler.post(() -> progress.onProgress(bytesDownloaded, totalBytes));
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
        if (totalBytes > 0 && sz > 0) {
            bytesDownloaded += sz;
            Networking.mainHandler.post(() -> progress.onProgress(bytesDownloaded, totalBytes));
        }
        return sz;
    }
}
