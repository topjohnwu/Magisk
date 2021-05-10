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
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ktx.createClassLoader
import com.topjohnwu.magisk.ktx.reflectField
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
import timber.log.Timber
import java.io.ByteArrayInputStream
import java.io.File
import java.io.IOException
import java.lang.reflect.InvocationHandler
import java.lang.reflect.Proxy
import java.security.SecureRandom
import java.security.Signature

class CheckSafetyNetEvent(
    private val callback: (SafetyNetResult) -> Unit = {}
) : ViewEventWithScope(), ContextExecutor, SafetyNetHelper.Callback {

    private val svc get() = ServiceLocator.networkService

    private lateinit var jar: File
    private lateinit var nonce: ByteArray

    override fun invoke(context: Context) {
        jar = File("${context.filesDir.parent}/snet", "snet.jar")

        scope.launch(Dispatchers.IO) {
            attest(context) {
                // Download and retry
                Shell.sh("rm -rf " + jar.parent).exec()
                jar.parentFile?.mkdir()
                withContext(Dispatchers.Main) {
                    showDialog(context)
                }
            }
        }
    }

    private suspend fun attest(context: Context, onError: suspend (Exception) -> Unit) {
        val helper: SafetyNetHelper
        try {
            val loader = createClassLoader(jar)

            // Scan through the dex and find our helper class
            var clazz: Class<*>? = null
            loop@for (dex in loader.getDexFiles()) {
                for (name in dex.entries()) {
                    val cls = loader.loadClass(name)
                    if (InvocationHandler::class.java.isAssignableFrom(cls)) {
                        clazz = cls
                        break@loop
                    }
                }
            }
            clazz ?: throw Exception("Cannot find SafetyNetHelper implementation")

            helper = Proxy.newProxyInstance(
                loader, arrayOf(SafetyNetHelper::class.java),
                clazz.newInstance() as InvocationHandler) as SafetyNetHelper

            if (helper.version != Const.SNET_EXT_VER)
                throw Exception("snet extension version mismatch")
        } catch (e: Exception) {
            onError(e)
            return
        }

        val random = SecureRandom()
        nonce = ByteArray(24)
        random.nextBytes(nonce)
        helper.attest(context, nonce, this)
    }

    // All of these fields are whitelisted
    private fun BaseDexClassLoader.getDexFiles(): List<DexFile> {
        val pathList = BaseDexClassLoader::class.java.reflectField("pathList").get(this)
        val dexElements = pathList.javaClass.reflectField("dexElements").get(pathList) as Array<*>
        val fileField = dexElements.javaClass.componentType.reflectField("dexFile")
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
            svc.fetchSafetynet().byteStream().writeTo(jar)
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
        return if (contains("[+/]".toRegex()))
            Base64.decode(this, Base64.DEFAULT)
        else
            Base64.decode(this, Base64.URL_SAFE)
    }

    private fun String.parseJws(): SafetyNetResponse {
        val jws = split('.')
        val secondDot = lastIndexOf('.')
        val rawHeader = String(jws[0].decode())
        val payload = String(jws[1].decode())
        var signature = jws[2].decode()
        val signedBytes = substring(0, secondDot).toByteArray()

        val moshi = Moshi.Builder().build()
        val header = moshi.adapter(JwsHeader::class.java).fromJson(rawHeader)
            ?: error("Invalid JWS header")

        val alg = when (header.algorithm) {
            "RS256" -> "SHA256withRSA"
            "ES256" -> {
                // Convert to DER encoding
                signature = ASN1Primitive.fromByteArray(signature).getEncoded(ASN1Encoding.DER)
                "SHA256withECDSA"
            }
            else -> error("Unsupported algorithm: ${header.algorithm}")
        }

        // Verify signature
        val certB64 = header.certificates?.first() ?: error("Cannot find certificate in JWS")
        val bis = ByteArrayInputStream(certB64.decode())
        val cert = CryptoUtils.readCertificate(bis)
        val verifier = Signature.getInstance(alg)
        verifier.initVerify(cert.publicKey)
        verifier.update(signedBytes)
        if (!verifier.verify(signature))
            error("Signature mismatch")

        // Verify hostname
        val hostnameVerifier = JsseDefaultHostnameAuthorizer(setOf())
        if (!hostnameVerifier.verify("attest.android.com", cert))
            error("Hostname mismatch")

        val response = moshi.adapter(SafetyNetResponse::class.java).fromJson(payload)
            ?: error("Invalid SafetyNet response")

        // Verify results
        if (!response.nonce.decode().contentEquals(nonce))
            error("nonce mismatch")

        return response
    }

    override fun onResponse(response: String?) {
        if (response != null) {
            scope.launch(Dispatchers.Default) {
                val res = runCatching { response.parseJws() }.getOrElse {
                    Timber.e(it)
                    INVALID_RESPONSE
                }
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

// Special instance to indicate invalid SafetyNet response
val INVALID_RESPONSE = SafetyNetResponse("", ctsProfileMatch = false, basicIntegrity = false)
