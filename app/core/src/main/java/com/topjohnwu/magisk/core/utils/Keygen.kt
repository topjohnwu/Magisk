package com.topjohnwu.magisk.core.utils

import android.util.Base64
import android.util.Base64OutputStream
import com.topjohnwu.magisk.core.Config
import org.bouncycastle.asn1.x500.X500Name
import org.bouncycastle.asn1.x509.SubjectPublicKeyInfo
import org.bouncycastle.cert.X509v3CertificateBuilder
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder
import java.io.ByteArrayOutputStream
import java.math.BigInteger
import java.security.KeyPairGenerator
import java.security.KeyStore
import java.security.PrivateKey
import java.security.cert.X509Certificate
import java.util.Calendar
import java.util.Locale
import java.util.Random
import java.util.zip.GZIPInputStream
import java.util.zip.GZIPOutputStream

private interface CertKeyProvider {
    val cert: X509Certificate
    val key: PrivateKey
}

class Keygen : CertKeyProvider {

    companion object {
        private const val ALIAS = "magisk"
        private val PASSWORD get() = "magisk".toCharArray()
        private const val DNAME = "C=US,ST=California,L=Mountain View,O=Google Inc.,OU=Android,CN=Android"
        private const val BASE64_FLAG = Base64.NO_PADDING or Base64.NO_WRAP
    }

    private val start = Calendar.getInstance().apply { add(Calendar.MONTH, -3) }
    private val end = (start.clone() as Calendar).apply { add(Calendar.YEAR, 30) }

    private val ks = init()
    override val cert = ks.getCertificate(ALIAS) as X509Certificate
    override val key = ks.getKey(ALIAS, PASSWORD) as PrivateKey

    private fun init(): KeyStore {
        val raw = Config.keyStoreRaw
        val ks = KeyStore.getInstance("PKCS12")
        if (raw.isEmpty()) {
            ks.load(null)
        } else {
            GZIPInputStream(Base64.decode(raw, BASE64_FLAG).inputStream()).use {
                ks.load(it, PASSWORD)
            }
        }

        // Keys already exist
        if (ks.containsAlias(ALIAS))
            return ks

        // Generate new private key and certificate
        val kp = KeyPairGenerator.getInstance("RSA").apply { initialize(4096) }.genKeyPair()
        val dname = X500Name(DNAME)
        val builder = X509v3CertificateBuilder(
            dname, BigInteger(160, Random()),
            start.time, end.time, Locale.ROOT, dname,
            SubjectPublicKeyInfo.getInstance(kp.public.encoded)
        )
        val signer = JcaContentSignerBuilder("SHA1WithRSA").build(kp.private)
        val cert = JcaX509CertificateConverter().getCertificate(builder.build(signer))

        // Store them into keystore
        ks.setKeyEntry(ALIAS, kp.private, PASSWORD, arrayOf(cert))
        val bytes = ByteArrayOutputStream()
        GZIPOutputStream(Base64OutputStream(bytes, BASE64_FLAG)).use {
            ks.store(it, PASSWORD)
        }
        Config.keyStoreRaw = bytes.toString("UTF-8")

        return ks
    }
}
