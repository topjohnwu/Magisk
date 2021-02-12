package com.topjohnwu.magisk.net;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

public class NoSSLv3SocketFactory extends SSLSocketFactory {

    private final static SSLSocketFactory delegate = HttpsURLConnection.getDefaultSSLSocketFactory();

    @Override
    public String[] getDefaultCipherSuites() {
        return delegate.getDefaultCipherSuites();
    }

    @Override
    public String[] getSupportedCipherSuites() {
        return delegate.getSupportedCipherSuites();
    }

    private Socket createSafeSocket(Socket socket) {
        if (socket instanceof SSLSocket)
            return new SSLSocketWrapper((SSLSocket) socket) {
                @Override
                public void setEnabledProtocols(String[] protocols) {
                    List<String> proto = new ArrayList<>(Arrays.asList(getSupportedProtocols()));
                    proto.remove("SSLv3");
                    super.setEnabledProtocols(proto.toArray(new String[0]));
                }
            };
        return socket;
    }

    @Override
    public Socket createSocket(Socket s, String host, int port, boolean autoClose) throws IOException {
        return createSafeSocket(delegate.createSocket(s, host, port, autoClose));
    }

    @Override
    public Socket createSocket() throws IOException {
        return createSafeSocket(delegate.createSocket());
    }

    @Override
    public Socket createSocket(String host, int port) throws IOException {
        return createSafeSocket(delegate.createSocket(host, port));
    }

    @Override
    public Socket createSocket(String host, int port, InetAddress localHost, int localPort) throws IOException {
        return createSafeSocket(delegate.createSocket(host, port, localHost, localPort));
    }

    @Override
    public Socket createSocket(InetAddress host, int port) throws IOException {
        return createSafeSocket(delegate.createSocket(host, port));
    }

    @Override
    public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort) throws IOException {
        return createSafeSocket(delegate.createSocket(address, port, localAddress, localPort));
    }
}
