package com.topjohnwu.magisk.utils;

import android.text.TextUtils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.Collections;

/**
 * Modified by topjohnwu, based on Chainfire's libsuperuser
 */

public class StreamGobbler extends Thread {

    private BufferedReader reader;
    private Collection<String> writer;

    /**
     * <p>StreamGobbler constructor</p>
     *
     * <p>We use this class because sh STDOUT and STDERR should be read as quickly as
     * possible to prevent a deadlock from occurring, or Process.waitFor() never
     * returning (as the buffer is full, pausing the native process)</p>
     *
     * @param in InputStream to read from
     * @param out  {@literal List<String>} to write to, or null
     */
    public StreamGobbler(InputStream in, Collection<String> out) {
        try {
            while (in.available() != 0) {
                in.skip(in.available());
            }
        } catch (IOException ignored) {}
        reader = new BufferedReader(new InputStreamReader(in));
        writer = out == null ? null : Collections.synchronizedCollection(out);
    }

    @Override
    public void run() {
        // keep reading the InputStream until it ends (or an error occurs)
        try {
            String line;
            while ((line = reader.readLine()) != null) {
                if (TextUtils.equals(line, "-shell-done-"))
                    return;
                if (writer != null) writer.add(line);
                Logger.shell(false, line);
            }
        } catch (IOException e) {
            // reader probably closed, expected exit condition
        }

        // make sure our stream is closed and resources will be freed
        try {
            reader.close();
        } catch (IOException e) {
            // read already closed
        }
    }
}
