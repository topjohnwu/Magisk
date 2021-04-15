@file:Suppress("DEPRECATION")

package com.topjohnwu.magisk.ui.safetynet

import android.content.Context
import android.util.Base64
import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass
import com.squareup.moshi.Moshi
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.ViewEventWithScope
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.ktx.DynamicClassLoader
import com.topjohnwu.magisk.ktx.writeTo
import com.topjohnwu.magisk.view.MagiskDialog
import com.topjohnwu.signing.CryptoUtils
import com.topjohnwu.superuser.Shell
import dalvik.system.BaseDexClassLoader
import dalvik.system.DexFile
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.bouncycastle.asn1.ASN1Encoding
import org.bouncycastle.asn1.ASN1Primitive
import org.bouncycastle.est.jcajce.JsseDefaultHostnameAuthorizer
import org.koin.core.KoinComponent
import org.koin.core.inject
import timber.log.Timber
import java.io.ByteArrayInputStream
import java.io.File
import java.io.IOException
import java.lang.reflect.Field
import java.lang.reflect.InvocationHandler
import java.security.GeneralSecurityException
import java.security.SecureRandom
import java.security.Signature
import java.security.cert.X509Certificate

class CheckSafetyNetEvent(
    private val callback: (SafetyNetResult) -> Unit = {}
) : ViewEventWithScope(), ContextExecutor, KoinComponent, SafetyNetHelper.Callback {

    private val svc by inject<NetworkService>()

    private lateinit var apk: File
    private lateinit var dex: File
    private lateinit var nonce: ByteArray

    override fun invoke(context: Context) {
        apk = File("${context.filesDir.parent}/snet", "snet.jar")
        dex = File(apk.parent, "snet.dex")

        scope.launch(Dispatchers.IO) {
            attest(context) {
                // Download and retry
                Shell.sh("rm -rf " + apk.parent).exec()
                apk.parentFile?.mkdir()
                withContext(Dispatchers.Main) {
                    showDialog(context)
                }
            }
        }
    }

    private suspend fun attest(context: Context, onError: suspend (Exception) -> Unit) {
        val helper: SafetyNetHelper
        try {
            val loader = DynamicClassLoader(apk)

            // Scan through the dex and find our helper class
            var clazz: Class<*>? = null
            loop@for (dex in loader.getDexFiles()) {
                for (name in dex.entries()) {
                    if (name.startsWith("x.")) {
                        val cls = loader.loadClass(name)
                        if (InvocationHandler::class.java.isAssignableFrom(cls)) {
                            clazz = cls
                            break@loop
                        }
                    }
                }
            }
            clazz ?: throw Exception("Cannot find SafetyNetHelper class")

            helper = clazz.getMethod("get", Class::class.java, Context::class.java, Any::class.java)
                .invoke(null, SafetyNetHelper::class.java, context, this) as SafetyNetHelper

            if (helper.version != Const.SNET_EXT_VER)
                throw Exception("snet extension version mismatch")
        } catch (e: Exception) {
            onError(e)
            return
        }

        val random = SecureRandom()
        nonce = ByteArray(24)
        random.nextBytes(nonce)
        helper.attest(nonce)
    }

    private fun Class<*>.field(name: String): Field =
        getDeclaredField(name).apply { isAccessible = true }

    // All of these fields are whitelisted
    private fun BaseDexClassLoader.getDexFiles(): List<DexFile> {
        val pathList = BaseDexClassLoader::class.java.field("pathList").get(this)
        val dexElements = pathList.javaClass.field("dexElements").get(pathList) as Array<*>
        val fileField = dexElements.javaClass.componentType.field("dexFile")
        return dexElements.map { fileField.get(it) as DexFile }
    }

    private fun download(context: Context) = scope.launch(Dispatchers.IO) {
        val abort: suspend (Exception) -> Unit = {
            Timber.e(it)
            withContext(Dispatchers.Main) {
                callback(SafetyNetResult())
            }
        }
        try {
            svc.fetchSafetynet().byteStream().writeTo(apk)
            attest(context, abort)
        } catch (e: IOException) {
            abort(e)
        }
    }

    private fun showDialog(context: Context) {
        MagiskDialog(context)
            .applyTitle(R.string.proprietary_title)
            .applyMessage(R.string.proprietary_notice)
            .cancellable(false)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = android.R.string.ok
                onClick { download(context) }
            }
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = android.R.string.cancel
                onClick { callback(SafetyNetResult(dismiss = true)) }
            }
            .onCancel {
                callback(SafetyNetResult(dismiss = true))
            }
            .reveal()
    }

    private fun String.decode(): ByteArray {
        return if (contains("\\+|/".toRegex()))
            Base64.decode(this, Base64.DEFAULT)
        else
            Base64.decode(this, Base64.URL_SAFE)
    }

    private fun String.parseJws(): SafetyNetResponse? {
        val jws = split('.')
        val secondDot = lastIndexOf('.')
        val rawHeader = String(jws[0].decode())
        val payload = String(jws[1].decode())
        var signature = jws[2].decode()
        val signedBytes = substring(0, secondDot).toByteArray()

        val moshi = Moshi.Builder().build()
        val header = moshi.adapter(JwsHeader::class.java).fromJson(rawHeader) ?: return null

        val alg = when (header.algorithm) {
            "RS256" -> "SHA256withRSA"
            "ES256" -> {
                // Convert to DER encoding
                signature = ASN1Primitive.fromByteArray(signature).getEncoded(ASN1Encoding.DER)
                "SHA256withECDSA"
            }
            else -> return null
        }

        // Verify signature
        val certB64 = header.certificates?.first() ?: return null
        val certDer = certB64.decode()
        val bis = ByteArrayInputStream(certDer)
        val cert: X509Certificate
        try {
            cert = CryptoUtils.readCertificate(bis)
            val verifier = Signature.getInstance(alg)
            verifier.initVerify(cert.publicKey)
            verifier.update(signedBytes)
            if (!verifier.verify(signature))
                return null
        } catch (e: GeneralSecurityException) {
            Timber.e(e)
            return null
        }

        // Verify hostname
        val hostNameVerifier = JsseDefaultHostnameAuthorizer(setOf())
        try {
            if (!hostNameVerifier.verify("attest.android.com", cert))
                return null
        } catch (e: IOException) {
            Timber.e(e)
            return null
        }

        val response = moshi.adapter(SafetyNetResponse::class.java).fromJson(payload) ?: return null

        // Verify results
        if (!response.nonce.decode().contentEquals(nonce))
            return null

        return response
    }

    override fun onResponse(response: String?) {
        if (response != null) {
            scope.launch(Dispatchers.Default) {
                val res = response.parseJws()
                withContext(Dispatchers.Main) {
                    callback(SafetyNetResult(res))
                }
            }
        } else {
            callback(SafetyNetResult())
        }
    }
}

@JsonClass(generateAdapter = true)
data class JwsHeader(
    @Json(name = "alg") val algorithm: String,
    @Json(name = "x5c") val certificates: List<String>?
)

@JsonClass(generateAdapter = true)
data class SafetyNetResponse(
    val nonce: String,
    val ctsProfileMatch: Boolean,
    val basicIntegrity: Boolean,
    val evaluationType: String = ""
)
