package com.topjohnwu.magisk.core.utils;

import org.apache.commons.io.input.BoundedInputStream;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.FileChannel;
import java.nio.channels.NonWritableChannelException;
import java.nio.channels.SeekableByteChannel;

import okhttp3.OkHttpClient;
import okhttp3.Request;

public class DataSourceChannel implements SeekableByteChannel {
    private static final int RANDOM_READ_CACHE_SIZE = 16 * 1024;
    private static final int SEQ_READ_CACHE_SIZE = 1024 * 1024;
    private static final int SEQ_READ_THRESHOLD = 1024;
    private static final int DIRECT_READ_THRESHOLD = 512 * 1024;

    private final OkHttpClient client;
    private final String url;
    private final FileChannel fileChannel;
    private final long startOffset;
    private final long size;

    private long position = 0;
    private boolean open = true;

    private byte[] cache = null;
    private long cacheStart = -1;

    private DataSourceChannel(OkHttpClient client, String url, FileChannel fileChannel,
                              long startOffset, long size) {
        this.client = client;
        this.url = url;
        this.fileChannel = fileChannel;
        this.startOffset = startOffset;
        this.size = size;
    }

    public DataSourceChannel(FileChannel fileChannel) throws IOException {
        this(null, null, fileChannel, 0, fileChannel.size());
    }

    public DataSourceChannel(OkHttpClient client, String url) throws IOException {
        this(client, url, null, 0, fetchTotalSize(client, url));
    }

    private static long fetchTotalSize(OkHttpClient client, String url) throws IOException {
        var request = new Request.Builder().url(url).head().build();
        try (var response = client.newCall(request).execute()) {
            if (!response.isSuccessful()) {
                throw new IOException("Failed to connect to URL: " + response);
            }
            var contentLength = response.header("Content-Length");
            if (contentLength == null) {
                throw new IOException("Could not determine file size.");
            }
            var acceptRanges = response.header("Accept-Ranges");
            if (acceptRanges == null || !acceptRanges.equalsIgnoreCase("bytes")) {
                throw new IOException("Server does not support byte ranges: " + response);
            }
            return Long.parseLong(contentLength);
        }
    }

    public DataSourceChannel slice(long offset, long sliceSize) {
        if (offset == 0 && sliceSize == size) {
            return this;
        }
        if (offset < 0 || sliceSize <= 0 || offset + sliceSize >= size) {
            throw new IllegalArgumentException("Invalid slice parameters");
        }
        return new DataSourceChannel(client, url, fileChannel, startOffset + offset, sliceSize);
    }

    @Override
    public int read(ByteBuffer dst) throws IOException {
        var bytesRead = read(dst, position);
        position += bytesRead;
        return bytesRead;
    }

    public int read(ByteBuffer dst, long position) throws IOException {
        if (!open) throw new ClosedChannelException();
        if (position < 0) {
            throw new IllegalArgumentException("Position out of bounds: " + position);
        }
        if (position >= size) return -1;

        int requestSize = dst.remaining();
        if (requestSize == 0) return 0;

        if (requestSize > DIRECT_READ_THRESHOLD) {
            return handleLargeRead(dst, position);
        }

        int totalBytesRead = 0;
        if (isCacheHit(position, 1)) {
            int bytesFromCache = readFromCache(dst, position);
            totalBytesRead += bytesFromCache;
            position += bytesFromCache;
        }

        if (dst.hasRemaining() && position < size) {
            loadCache(position, requestSize);
            if (isCacheHit(position, dst.remaining())) {
                totalBytesRead += readFromCache(dst, position);
            } else {
                totalBytesRead += readDirectly(dst, position);
            }
        }

        return totalBytesRead;
    }

    private int handleLargeRead(ByteBuffer dst, long position) throws IOException {
        int bytesFromCache = 0;
        if (isCacheHit(position, 1)) {
            bytesFromCache = readFromCache(dst, position);
            position += bytesFromCache;
        }

        if (dst.hasRemaining() && position < size) {
            int directBytesRead = readDirectly(dst, position);
            return bytesFromCache + directBytesRead;
        } else {
            return bytesFromCache;
        }
    }

    private void loadCache(long requestPos, int requestSize) throws IOException {
        int cacheSize;
        long cacheStart;

        var lastCacheEnd = cache != null ? this.cacheStart + cache.length : -1;
        if (requestSize > SEQ_READ_THRESHOLD || lastCacheEnd == requestPos) {
            cacheSize = SEQ_READ_CACHE_SIZE;
            cacheStart = requestPos;
        } else {
            cacheSize = RANDOM_READ_CACHE_SIZE;
            cacheStart = Math.max(0, requestPos - cacheSize / 2);
        }

        loadCacheAt(cacheStart, cacheSize);
    }

    private void loadCacheAt(long cacheStart, int cacheSize) throws IOException {
        long maxEnd = Math.min(cacheStart + cacheSize, size);
        cacheStart = Math.max(0, maxEnd - cacheSize);

        var buffer = ByteBuffer.allocate((int) (maxEnd - cacheStart));
        var bytesRead = readDirectly(buffer, cacheStart);
        if (bytesRead != buffer.capacity()) {
            throw new IOException("Failed to fill cache.");
        }

        cache = buffer.array();
        this.cacheStart = cacheStart;

    }

    private boolean isCacheHit(long pos, int bytesToRead) {
        if (cache == null) return false;
        long cacheEnd = cacheStart + cache.length;
        long readEnd = Math.min(pos + bytesToRead, size);
        return pos >= cacheStart && readEnd <= cacheEnd;
    }

    private int readFromCache(ByteBuffer dst, long position) {
        long relativePos = position - cacheStart;
        int available = (int) Math.min(dst.remaining(), cache.length - relativePos);

        dst.put(cache, (int) relativePos, available);

        return available;
    }

    private int readDirectly(ByteBuffer dst, long position) throws IOException {
        try (var channel = Channels.newChannel(streamRead(position, dst.remaining()))) {
            int totalBytesRead = 0;
            while (true) {
                int bytesRead = channel.read(dst);
                if (bytesRead <= 0) {
                    break;
                }
                totalBytesRead += bytesRead;
            }

            return totalBytesRead;
        }
    }

    public InputStream streamRead(long position, long length) throws IOException {
        long endPosition = Math.min(position + length, size) + startOffset;
        var startPosition = startOffset + position;
        var readLength = endPosition - startPosition;

        if (fileChannel != null) {
            fileChannel.position(startPosition);
            return BoundedInputStream.builder()
                    .setInputStream(Channels.newInputStream(fileChannel))
                    .setMaxCount(readLength)
                    .setPropagateClose(false)
                    .get();
        }

        var request = new Request.Builder()
                .url(url)
                .header("Range", "bytes=" + startPosition + "-" + (endPosition - 1))
                .build();

        var response = client.newCall(request).execute();
        if (response.code() != 206) {
            response.close();
            throw new IOException("Unexpected response code " + response.code());
        }
        return response.body().byteStream();
    }

    @Override
    public long position() {
        return position;
    }

    @Override
    public DataSourceChannel position(long newPosition) throws IOException {
        if (!open) throw new ClosedChannelException();
        if (newPosition < 0) {
            throw new IllegalArgumentException("Position out of bounds: " + newPosition);
        }
        position = newPosition;
        return this;
    }

    @Override
    public long size() {
        return size;
    }

    @Override
    public boolean isOpen() {
        return open;
    }

    @Override
    public void close() throws IOException {
        open = false;
        cache = null;
        if (fileChannel != null) {
            fileChannel.close();
        }
    }

    @Override
    public int write(ByteBuffer src) {
        throw new NonWritableChannelException();
    }

    @Override
    public DataSourceChannel truncate(long size) {
        throw new NonWritableChannelException();
    }
}
