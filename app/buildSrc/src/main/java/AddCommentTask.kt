import com.android.build.api.artifact.ArtifactTransformationRequest
import com.android.build.api.dsl.ApkSigningConfig
import com.android.builder.internal.packaging.IncrementalPackager
import com.android.tools.build.apkzlib.sign.SigningExtension
import com.android.tools.build.apkzlib.sign.SigningOptions
import com.android.tools.build.apkzlib.zfile.ZFiles
import com.android.tools.build.apkzlib.zip.ZFileOptions
import org.gradle.api.DefaultTask
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.Internal
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.TaskAction
import java.io.File
import java.security.KeyStore
import java.security.cert.X509Certificate
import java.util.jar.JarFile

abstract class AddCommentTask: DefaultTask() {
    @get:Input
    abstract val comment: Property<String>

    @get:Input
    abstract val signingConfig: Property<ApkSigningConfig>

    @get:InputFiles
    abstract val apkFolder: DirectoryProperty

    @get:OutputDirectory
    abstract val outFolder: DirectoryProperty

    @get:Internal
    abstract val transformationRequest: Property<ArtifactTransformationRequest<AddCommentTask>>

    @TaskAction
    fun taskAction() = transformationRequest.get().submit(this) { artifact ->
        val inFile = File(artifact.outputFile)
        val outFile = outFolder.file(inFile.name).get().asFile

        val privateKey = signingConfig.get().getPrivateKey()
        val signingOptions = SigningOptions.builder()
            .setMinSdkVersion(0)
            .setV1SigningEnabled(true)
            .setV2SigningEnabled(true)
            .setKey(privateKey.privateKey)
            .setCertificates(privateKey.certificate as X509Certificate)
            .setValidation(SigningOptions.Validation.ASSUME_INVALID)
            .build()
        val options = ZFileOptions().apply {
            noTimestamps = true
            autoSortFiles = true
        }
        outFile.parentFile?.mkdirs()
        inFile.copyTo(outFile, overwrite = true)
        ZFiles.apk(outFile, options).use {
            SigningExtension(signingOptions).register(it)
            it.eocdComment = comment.get().toByteArray()
            it.get(IncrementalPackager.APP_METADATA_ENTRY_PATH)?.delete()
            it.get(IncrementalPackager.VERSION_CONTROL_INFO_ENTRY_PATH)?.delete()
            it.get(JarFile.MANIFEST_NAME)?.delete()
        }

        outFile
    }

    private fun ApkSigningConfig.getPrivateKey(): KeyStore.PrivateKeyEntry {
        val keyStore = KeyStore.getInstance(storeType ?: KeyStore.getDefaultType())
        storeFile!!.inputStream().use {
            keyStore.load(it, storePassword!!.toCharArray())
        }
        val keyPwdArray = keyPassword!!.toCharArray()
        val entry = keyStore.getEntry(keyAlias!!, KeyStore.PasswordProtection(keyPwdArray))
        return entry as KeyStore.PrivateKeyEntry
    }
}