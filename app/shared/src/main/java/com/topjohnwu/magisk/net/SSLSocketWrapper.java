package com.topjohnwu.magisk.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.nio.channels.SocketChannel;

import javax.net.ssl.HandshakeCompletedListener;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocket;

class SSLSocketWrapper extends SSLSocket {

    private SSLSocket mBase;

    SSLSocketWrapper(SSLSocket socket) {
        mBase = socket;
    }

    @Override
    public String[] getSupportedCipherSuites() {
        return mBase.getSupportedCipherSuites();
    }

    @Override
    public String[] getEnabledCipherSuites() {
        return mBase.getEnabledCipherSuites();
    }

    @Override
    public void setEnabledCipherSuites(String[] suites) {
        mBase.setEnabledCipherSuites(suites);
    }

    @Override
    public String[] getSupportedProtocols() {
        return mBase.getSupportedProtocols();
    }

    @Override
    public String[] getEnabledProtocols() {
        return mBase.getEnabledProtocols();
    }

    @Override
    public void setEnabledProtocols(String[] protocols) {
        mBase.setEnabledProtocols(protocols);
    }

    @Override
    public SSLSession getSession() {
        return mBase.getSession();
    }

    @Override
    public SSLSession getHandshakeSession() {
        throw new UnsupportedOperationException();
    }

    @Override
    public void addHandshakeCompletedListener(HandshakeCompletedListener listener) {
        mBase.addHandshakeCompletedListener(listener);
    }

    @Override
    public void removeHandshakeCompletedListener(HandshakeCompletedListener listener) {
        mBase.removeHandshakeCompletedListener(listener);
    }

    @Override
    public void startHandshake() throws IOException {
        mBase.startHandshake();
    }

    @Override
    public void setUseClientMode(boolean mode) {
        mBase.setUseClientMode(mode);
    }

    @Override
    public boolean getUseClientMode() {
        return mBase.getUseClientMode();
    }

    @Override
    public void setNeedClientAuth(boolean need) {
        mBase.setNeedClientAuth(need);
    }

    @Override
    public boolean getNeedClientAuth() {
        return mBase.getNeedClientAuth();
    }

    @Override
    public void setWantClientAuth(boolean want) {
        mBase.setWantClientAuth(want);
    }

    @Override
    public boolean getWantClientAuth() {
        return mBase.getWantClientAuth();
    }

    @Override
    public void setEnableSessionCreation(boolean flag) {
        mBase.setEnableSessionCreation(flag);
    }

    @Override
    public boolean getEnableSessionCreation() {
        return mBase.getEnableSessionCreation();
    }

    @Override
    public SSLParameters getSSLParameters() {
        return mBase.getSSLParameters();
    }

    @Override
    public void setSSLParameters(SSLParameters params) {
        mBase.setSSLParameters(params);
    }

    @Override
    public String toString() {
        return mBase.toString();
    }

    @Override
    public void connect(SocketAddress endpoint) throws IOException {
        mBase.connect(endpoint);
    }

    @Override
    public void connect(SocketAddress endpoint, int timeout) throws IOException {
        mBase.connect(endpoint, timeout);
    }

    @Override
    public void bind(SocketAddress bindpoint) throws IOException {
        mBase.bind(bindpoint);
    }

    @Override
    public InetAddress getInetAddress() {
        return mBase.getInetAddress();
    }

    @Override
    public InetAddress getLocalAddress() {
        return mBase.getLocalAddress();
    }

    @Override
    public int getPort() {
        return mBase.getPort();
    }

    @Override
    public int getLocalPort() {
        return mBase.getLocalPort();
    }

    @Override
    public SocketAddress getRemoteSocketAddress() {
        return mBase.getRemoteSocketAddress();
    }

    @Override
    public SocketAddress getLocalSocketAddress() {
        return mBase.getLocalSocketAddress();
    }

    @Override
    public SocketChannel getChannel() {
        return mBase.getChannel();
    }

    @Override
    public InputStream getInputStream() throws IOException {
        return mBase.getInputStream();
    }

    @Override
    public OutputStream getOutputStream() throws IOException {
        return mBase.getOutputStream();
    }

    @Override
    public void setTcpNoDelay(boolean on) throws SocketException {
        mBase.setTcpNoDelay(on);
    }

    @Override
    public boolean getTcpNoDelay() throws SocketException {
        return mBase.getTcpNoDelay();
    }

    @Override
    public void setSoLinger(boolean on, int linger) throws SocketException {
        mBase.setSoLinger(on, linger);
    }

    @Override
    public int getSoLinger() throws SocketException {
        return mBase.getSoLinger();
    }

    @Override
    public void sendUrgentData(int data) throws IOException {
        mBase.sendUrgentData(data);
    }

    @Override
    public void setOOBInline(boolean on) throws SocketException {
        mBase.setOOBInline(on);
    }

    @Override
    public boolean getOOBInline() throws SocketException {
        return mBase.getOOBInline();
    }

    @Override
    public void setSoTimeout(int timeout) throws SocketException {
        mBase.setSoTimeout(timeout);
    }

    @Override
    public int getSoTimeout() throws SocketException {
        return mBase.getSoTimeout();
    }

    @Override
    public void setSendBufferSize(int size) throws SocketException {
        mBase.setSendBufferSize(size);
    }

    @Override
    public int getSendBufferSize() throws SocketException {
        return mBase.getSendBufferSize();
    }

    @Override
    public void setReceiveBufferSize(int size) throws SocketException {
        mBase.setReceiveBufferSize(size);
    }

    @Override
    public int getReceiveBufferSize() throws SocketException {
        return mBase.getReceiveBufferSize();
    }

    @Override
    public void setKeepAlive(boolean on) throws SocketException {
        mBase.setKeepAlive(on);
    }

    @Override
    public boolean getKeepAlive() throws SocketException {
        return mBase.getKeepAlive();
    }

    @Override
    public void setTrafficClass(int tc) throws SocketException {
        mBase.setTrafficClass(tc);
    }

    @Override
    public int getTrafficClass() throws SocketException {
        return mBase.getTrafficClass();
    }

    @Override
    public void setReuseAddress(boolean on) throws SocketException {
        mBase.setReuseAddress(on);
    }

    @Override
    public boolean getReuseAddress() throws SocketException {
        return mBase.getReuseAddress();
    }

    @Override
    public void close() throws IOException {
        mBase.close();
    }

    @Override
    public void shutdownInput() throws IOException {
        mBase.shutdownInput();
    }

    @Override
    public void shutdownOutput() throws IOException {
        mBase.shutdownOutput();
    }

    @Override
    public boolean isConnected() {
        return mBase.isConnected();
    }

    @Override
    public boolean isBound() {
        return mBase.isBound();
    }

    @Override
    public boolean isClosed() {
        return mBase.isClosed();
    }

    @Override
    public boolean isInputShutdown() {
        return mBase.isInputShutdown();
    }

    @Override
    public boolean isOutputShutdown() {
        return mBase.isOutputShutdown();
    }

    @Override
    public void setPerformancePreferences(int connectionTime, int latency, int bandwidth) {
        mBase.setPerformancePreferences(connectionTime, latency, bandwidth);
    }
}
