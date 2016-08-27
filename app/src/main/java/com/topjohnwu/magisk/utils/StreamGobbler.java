package com.topjohnwu.magisk.utils;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;

/**
 * Modified by topjohnwu, based on Chainfire's libsuperuser
 */

public class StreamGobbler extends Thread {

    private BufferedReader reader = null;
    private List<String> writer = null;

    /**
     * <p>StreamGobbler constructor</p>
     *
     * <p>We use this class because shell STDOUT and STDERR should be read as quickly as
     * possible to prevent a deadlock from occurring, or Process.waitFor() never
     * returning (as the buffer is full, pausing the native process)</p>
     *
     * @param inputStream InputStream to read from
     * @param outputList  {@literal List<String>} to write to, or null
     */
    public StreamGobbler(InputStream inputStream, List<String> outputList) {
        reader = new BufferedReader(new InputStreamReader(inputStream));
        writer = outputList;
    }

    @Override
    public void run() {
        // keep reading the InputStream until it ends (or an error occurs)
        try {
            String line;
            while ((line = reader.readLine()) != null) {
                writer.add(line);
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
