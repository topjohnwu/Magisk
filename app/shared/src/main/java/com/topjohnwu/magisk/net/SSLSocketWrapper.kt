package com.topjohnwu.magisk.net

import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.net.InetAddress
import java.net.SocketAddress
import java.net.SocketException
import java.nio.channels.SocketChannel
import javax.net.ssl.HandshakeCompletedListener
import javax.net.ssl.SSLParameters
import javax.net.ssl.SSLSession
import javax.net.ssl.SSLSocket
import kotlin.jvm.Throws

open class SSLSocketWrapper(private val mBase: SSLSocket) : SSLSocket() {

    override fun getSupportedCipherSuites(): Array<String> {
        return mBase.supportedCipherSuites
    }

    override fun getEnabledCipherSuites(): Array<String> {
        return mBase.enabledCipherSuites
    }

    override fun setEnabledCipherSuites(suites: Array<String>) {
        mBase.enabledCipherSuites = suites
    }

    override fun getSupportedProtocols(): Array<String> {
        return mBase.supportedProtocols
    }

    override fun getEnabledProtocols(): Array<String> {
        return mBase.enabledProtocols
    }

    override fun setEnabledProtocols(protocols: Array<String>) {
        mBase.enabledProtocols = protocols
    }

    override fun getSession(): SSLSession {
        return mBase.session
    }

    override fun getHandshakeSession(): SSLSession {
        throw UnsupportedOperationException()
    }

    override fun addHandshakeCompletedListener(listener: HandshakeCompletedListener) {
        mBase.addHandshakeCompletedListener(listener)
    }

    override fun removeHandshakeCompletedListener(listener: HandshakeCompletedListener) {
        mBase.removeHandshakeCompletedListener(listener)
    }

    @Throws(IOException::class) override fun startHandshake() {
        mBase.startHandshake()
    }

    override fun setUseClientMode(mode: Boolean) {
        mBase.useClientMode = mode
    }

    override fun getUseClientMode(): Boolean {
        return mBase.useClientMode
    }

    override fun setNeedClientAuth(need: Boolean) {
        mBase.needClientAuth = need
    }

    override fun getNeedClientAuth(): Boolean {
        return mBase.needClientAuth
    }

    override fun setWantClientAuth(want: Boolean) {
        mBase.wantClientAuth = want
    }

    override fun getWantClientAuth(): Boolean {
        return mBase.wantClientAuth
    }

    override fun setEnableSessionCreation(flag: Boolean) {
        mBase.enableSessionCreation = flag
    }

    override fun getEnableSessionCreation(): Boolean {
        return mBase.enableSessionCreation
    }

    override fun getSSLParameters(): SSLParameters {
        return mBase.sslParameters
    }

    override fun setSSLParameters(params: SSLParameters) {
        mBase.sslParameters = params
    }

    override fun toString(): String {
        return mBase.toString()
    }

    @Throws(IOException::class) override fun connect(endpoint: SocketAddress) {
        mBase.connect(endpoint)
    }

    @Throws(IOException::class) override fun connect(endpoint: SocketAddress, timeout: Int) {
        mBase.connect(endpoint, timeout)
    }

    @Throws(IOException::class) override fun bind(bindpoint: SocketAddress) {
        mBase.bind(bindpoint)
    }

    override fun getInetAddress(): InetAddress {
        return mBase.inetAddress
    }

    override fun getLocalAddress(): InetAddress {
        return mBase.localAddress
    }

    override fun getPort(): Int {
        return mBase.port
    }

    override fun getLocalPort(): Int {
        return mBase.localPort
    }

    override fun getRemoteSocketAddress(): SocketAddress {
        return mBase.remoteSocketAddress
    }

    override fun getLocalSocketAddress(): SocketAddress {
        return mBase.localSocketAddress
    }

    override fun getChannel(): SocketChannel {
        return mBase.channel
    }

    @Throws(IOException::class) override fun getInputStream(): InputStream {
        return mBase.inputStream
    }

    @Throws(IOException::class) override fun getOutputStream(): OutputStream {
        return mBase.outputStream
    }

    @Throws(SocketException::class) override fun setTcpNoDelay(on: Boolean) {
        mBase.tcpNoDelay = on
    }

    @Throws(SocketException::class) override fun getTcpNoDelay(): Boolean {
        return mBase.tcpNoDelay
    }

    @Throws(SocketException::class) override fun setSoLinger(on: Boolean, linger: Int) {
        mBase.setSoLinger(on, linger)
    }

    @Throws(SocketException::class) override fun getSoLinger(): Int {
        return mBase.soLinger
    }

    @Throws(IOException::class) override fun sendUrgentData(data: Int) {
        mBase.sendUrgentData(data)
    }

    @Throws(SocketException::class) override fun setOOBInline(on: Boolean) {
        mBase.oobInline = on
    }

    @Throws(SocketException::class) override fun getOOBInline(): Boolean {
        return mBase.oobInline
    }

    @Throws(SocketException::class) override fun setSoTimeout(timeout: Int) {
        mBase.soTimeout = timeout
    }

    @Throws(SocketException::class) override fun getSoTimeout(): Int {
        return mBase.soTimeout
    }

    @Throws(SocketException::class) override fun setSendBufferSize(size: Int) {
        mBase.sendBufferSize = size
    }

    @Throws(SocketException::class) override fun getSendBufferSize(): Int {
        return mBase.sendBufferSize
    }

    @Throws(SocketException::class) override fun setReceiveBufferSize(size: Int) {
        mBase.receiveBufferSize = size
    }

    @Throws(SocketException::class) override fun getReceiveBufferSize(): Int {
        return mBase.receiveBufferSize
    }

    @Throws(SocketException::class) override fun setKeepAlive(on: Boolean) {
        mBase.keepAlive = on
    }

    @Throws(SocketException::class) override fun getKeepAlive(): Boolean {
        return mBase.keepAlive
    }

    @Throws(SocketException::class) override fun setTrafficClass(tc: Int) {
        mBase.trafficClass = tc
    }

    @Throws(SocketException::class) override fun getTrafficClass(): Int {
        return mBase.trafficClass
    }

    @Throws(SocketException::class) override fun setReuseAddress(on: Boolean) {
        mBase.reuseAddress = on
    }

    @Throws(SocketException::class) override fun getReuseAddress(): Boolean {
        return mBase.reuseAddress
    }

    @Throws(IOException::class) override fun close() {
        mBase.close()
    }

    @Throws(IOException::class) override fun shutdownInput() {
        mBase.shutdownInput()
    }

    @Throws(IOException::class) override fun shutdownOutput() {
        mBase.shutdownOutput()
    }

    override fun isConnected(): Boolean {
        return mBase.isConnected
    }

    override fun isBound(): Boolean {
        return mBase.isBound
    }

    override fun isClosed(): Boolean {
        return mBase.isClosed
    }

    override fun isInputShutdown(): Boolean {
        return mBase.isInputShutdown
    }

    override fun isOutputShutdown(): Boolean {
        return mBase.isOutputShutdown
    }

    override fun setPerformancePreferences(connectionTime: Int, latency: Int, bandwidth: Int) {
        mBase.setPerformancePreferences(connectionTime, latency, bandwidth)
    }
}
