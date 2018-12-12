package com.topjohnwu.net;

public interface DownloadProgressListener {
    void onProgress(long bytesDownloaded, long totalBytes);
}
