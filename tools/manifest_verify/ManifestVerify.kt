package tools.manifestverify

import kotlinx.cli.ArgParser
import kotlinx.cli.ArgType
import kotlinx.cli.default
import org.json.JSONObject
import java.io.File
import java.security.MessageDigest
import javax.crypto.Mac
import javax.crypto.spec.SecretKeySpec
import android.util.Base64
 
fun hexStringToByteArray(s: String): ByteArray {
    val len = s.length
    val data = ByteArray(len / 2)
    var i = 0
    while (i < len) {
        data[i / 2] =
            ((Character.digit(s[i], 16) shl 4) + Character.digit(s[i + 1], 16)).toByte()
        i += 2
    }
    return data
}

fun computeSha256(path: String): String {
    val md = MessageDigest.getInstance("SHA-256")
    val f = File(path)
    f.inputStream().use { ins ->
        val buf = ByteArray(32 * 1024)
        var r: Int
        while (ins.read(buf).also { r = it } != -1) {
            md.update(buf, 0, r)
        }
    }
    return md.digest().joinToString("") { "%02x".format(it) }
}

fun computeHmacSha256Base64(hexSha: String, keyBase64: String): String {
    val keyRaw = Base64.decode(keyBase64, Base64.NO_WRAP)
    val sk = SecretKeySpec(keyRaw, "HmacSHA256")
    val mac = Mac.getInstance("HmacSHA256")
    mac.init(sk)
    val macRes = mac.doFinal(hexStringToByteArray(hexSha))
    return Base64.encodeToString(macRes, Base64.NO_WRAP)
}

fun main(args: Array<String>) {
    val parser = ArgParser("manifest_verify")
    val mode by parser.option(ArgType.String, shortName = "m", description = "mode: local|device").default("local")
    val manifestPath by parser.option(ArgType.String, shortName = "f", description = "manifest json path").default("")
    val keyBase64 by parser.option(ArgType.String, shortName = "k", description = "key base64 (local mode)").default("")
    parser.parse(args)

    if (manifestPath.isEmpty()) {
        println("manifest_verify: manifest file required (--manifest)")
        kotlin.system.exitProcess(2)
    }
    val mf = File(manifestPath)
    if (!mf.exists()) {
        println("manifest_verify: manifest not found: $manifestPath")
        kotlin.system.exitProcess(3)
    }
    val json = JSONObject(mf.readText(Charsets.UTF_8))
    val backupPath = json.optString("backupPath", json.optString("artifact_path", ""))
    val expectedSha = json.optString("sha256", "")
    val expectedHmac = json.optString("hmac_b64", "")

    if (backupPath.isEmpty()) {
        println("manifest_verify: manifest has no backupPath/artifact_path")
        kotlin.system.exitProcess(4)
    }
    if (expectedSha.isEmpty()) {
        println("manifest_verify: manifest has no sha256")
        kotlin.system.exitProcess(5)
    }

    val actualSha = computeSha256(backupPath)
    if (!actualSha.equals(expectedSha, ignoreCase = true)) {
        println("manifest_verify: SHA mismatch: expected=$expectedSha actual=$actualSha")
        kotlin.system.exitProcess(6)
    } else {
        println("manifest_verify: SHA ok")
    }

    if (mode == "local") {
        if (keyBase64.isEmpty()) {
            println("manifest_verify: local mode requires --key-base64")
            kotlin.system.exitProcess(7)
        }
        if (expectedHmac.isEmpty()) {
            println("manifest_verify: manifest has no hmac_b64; nothing to verify")
            kotlin.system.exitProcess(0)
        }
        val computedB64 = computeHmacSha256Base64(actualSha, keyBase64)
        if (computedB64 == expectedHmac) {
            println("manifest_verify: HMAC ok")
            kotlin.system.exitProcess(0)
        } else {
            println("manifest_verify: HMAC mismatch: expected=$expectedHmac computed=$computedB64")
            kotlin.system.exitProcess(8)
        }
    } else  {
        println("manifest_verify: device mode not implemented in this CLI - use app verifier via adb if needed.")
        kotlin.system.exitProcess(9)
    }
}
