package com.topjohnwu.utils;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;

public class ReusableInputStream extends BufferedInputStream {

    public ReusableInputStream(InputStream in) {
        super(in);
        mark(Integer.MAX_VALUE);
    }

    public ReusableInputStream(InputStream in, int size) {
        super(in, size);
        mark(Integer.MAX_VALUE);
    }

    @Override
    public void close() throws IOException {
        /* Reset at close so we can reuse it */
        reset();
    }

    public void destroy() throws IOException {
        super.close();
    }
}
