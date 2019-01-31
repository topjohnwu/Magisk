package com.topjohnwu.magisk.utils;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.text.TextUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

public abstract class SuConnector {

    private LocalSocket socket;
    protected DataOutputStream out;
    protected DataInputStream in;

    protected SuConnector(String name) throws IOException {
        socket = new LocalSocket();
        socket.connect(new LocalSocketAddress(name, LocalSocketAddress.Namespace.ABSTRACT));
        out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream()));
        in = new DataInputStream(new BufferedInputStream(socket.getInputStream()));
    }

    private String readString() throws IOException {
        int len = in.readInt();
        byte[] buf = new byte[len];
        in.readFully(buf);
        return new String(buf, "UTF-8");
    }

    public Bundle readSocketInput() throws IOException {
        Bundle bundle = new Bundle();
        while (true) {
            String name = readString();
            if (TextUtils.equals(name, "eof"))
                break;
            bundle.putString(name, readString());
        }
        return bundle;
    }

    public void response() {
        try {
            onResponse();
            out.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            in.close();
            out.close();
            socket.close();
        } catch (IOException ignored) { }
    }

    protected abstract void onResponse() throws IOException;

}
