import com.android.build.api.artifact.ArtifactTransformationRequest
import com.android.build.api.artifact.SingleArtifact
import com.android.build.api.dsl.ApkSigningConfig
import com.android.build.api.instrumentation.FramesComputationMode.COMPUTE_FRAMES_FOR_INSTRUMENTED_METHODS
import com.android.build.api.instrumentation.InstrumentationScope
import com.android.build.api.variant.ApplicationAndroidComponentsExtension
import com.android.build.gradle.BaseExtension
import com.android.build.gradle.LibraryExtension
import com.android.build.gradle.internal.dsl.BaseAppModuleExtension
import com.android.builder.internal.packaging.IncrementalPackager
import com.android.tools.build.apkzlib.sign.SigningExtension
import com.android.tools.build.apkzlib.sign.SigningOptions
import com.android.tools.build.apkzlib.zfile.ZFiles
import com.android.tools.build.apkzlib.zip.ZFileOptions
import org.apache.tools.ant.filters.FixCrLfFilter
import org.gradle.api.Action
import org.gradle.api.DefaultTask
import org.gradle.api.JavaVersion
import org.gradle.api.Project
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Copy
import org.gradle.api.tasks.Delete
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.Internal
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.StopExecutionException
import org.gradle.api.tasks.Sync
import org.gradle.api.tasks.TaskAction
import org.gradle.kotlin.dsl.assign
import org.gradle.kotlin.dsl.exclude
import org.gradle.kotlin.dsl.filter
import org.gradle.kotlin.dsl.get
import org.gradle.kotlin.dsl.getValue
import org.gradle.kotlin.dsl.named
import org.gradle.kotlin.dsl.provideDelegate
import org.gradle.kotlin.dsl.register
import org.gradle.kotlin.dsl.registering
import org.gradle.kotlin.dsl.withType
import org.jetbrains.kotlin.gradle.dsl.JvmTarget
import org.jetbrains.kotlin.gradle.tasks.KotlinCompile
import java.io.ByteArrayOutputStream
import java.io.File
import java.net.URI
import java.security.KeyStore
import java.security.MessageDigest
import java.security.cert.X509Certificate
import java.util.HexFormat
import java.util.jar.JarFile
import java.util.zip.Deflater
import java.util.zip.DeflaterOutputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

private fun Project.androidBase(configure: Action<BaseExtension>) =
    extensions.configure("android", configure)

private fun Project.android(configure: Action<BaseAppModuleExtension>) =
    extensions.configure("android", configure)

private val Project.androidApp: BaseAppModuleExtension
    get() = extensions["android"] as BaseAppModuleExtension

private val Project.androidLib: LibraryExtension
    get() = extensions["android"] as LibraryExtension

private val Project.androidComponents
    get() = extensions.getByType(ApplicationAndroidComponentsExtension::class.java)

fun Project.setupCommon() {
    androidBase {
        compileSdkVersion(35)
        buildToolsVersion = "35.0.1"
        ndkPath = "$sdkDirectory/ndk/magisk"
        ndkVersion = "29.0.13113456"

        defaultConfig {
            minSdk = 23
        }

        compileOptions {
            sourceCompatibility = JavaVersion.VERSION_21
            targetCompatibility = JavaVersion.VERSION_21
        }

        packagingOptions {
            resources {
                excludes += arrayOf(
                    "/META-INF/*",
                    "/META-INF/versions/**",
                    "/org/bouncycastle/**",
                    "/org/apache/commons/**",
                    "/kotlin/**",
                    "/kotlinx/**",
                    "/okhttp3/**",
                    "/*.txt",
                    "/*.bin",
                    "/*.json",
                )
            }
        }
    }

    configurations.all {
        exclude("org.jetbrains.kotlin", "kotlin-stdlib-jdk7")
        exclude("org.jetbrains.kotlin", "kotlin-stdlib-jdk8")
    }

    tasks.withType<KotlinCompile> {
        compilerOptions {
            jvmTarget = JvmTarget.JVM_21
        }
    }
}

private fun Project.downloadFile(url: String, checksum: String): File {
    val file = layout.buildDirectory.file(checksum).get().asFile
    if (file.exists()) {
        val md = MessageDigest.getInstance("SHA-256")
        file.inputStream().use { md.update(it.readAllBytes()) }
        val hash = HexFormat.of().formatHex(md.digest())
        if (hash != checksum) {
            file.delete()
        }
    }
    if (!file.exists()) {
        file.parentFile.mkdirs()
        URI(url).toURL().openStream().use { dl ->
            file.outputStream().use {
                dl.copyTo(it)
            }
        }
    }
    return file
}

const val BUSYBOX_DOWNLOAD_URL =
    "https://github.com/topjohnwu/magisk-files/releases/download/files/busybox-1.36.1.1.zip"
const val BUSYBOX_ZIP_CHECKSUM =
    "b4d0551feabaf314e53c79316c980e8f66432e9fb91a69dbbf10a93564b40951"

fun Project.setupCoreLib() {
    setupCommon()

    val abiList = Config.abiList

    val syncLibs by tasks.registering(Sync::class) {
        into("src/main/jniLibs")
        for (abi in abiList) {
            into(abi) {
                from(rootProject.file("native/out/$abi")) {
                    include("magiskboot", "magiskinit", "magiskpolicy", "magisk", "libinit-ld.so")
                    rename { if (it.endsWith(".so")) it else "lib$it.so" }
                }
            }
        }
        onlyIf {
            if (inputs.sourceFiles.files.size != abiList.size * 5)
                throw StopExecutionException("Please build binaries first! (./build.py binary)")
            true
        }
    }

    val downloadBusybox by tasks.registering(Copy::class) {
        dependsOn(syncLibs)
        from(zipTree(downloadFile(BUSYBOX_DOWNLOAD_URL, BUSYBOX_ZIP_CHECKSUM)))
        include(abiList.map { "$it/libbusybox.so" })
        into("src/main/jniLibs")
    }

    val syncResources by tasks.registering(Sync::class) {
        into("src/main/resources/META-INF/com/google/android")
        from(rootProject.file("scripts/update_binary.sh")) {
            rename { "update-binary" }
        }
        from(rootProject.file("scripts/flash_script.sh")) {
            rename { "updater-script" }
        }
    }

    androidLib.libraryVariants.all {
        val variantCapped = name.replaceFirstChar { it.uppercase() }

        tasks.getByPath("merge${variantCapped}JniLibFolders").dependsOn(downloadBusybox)
        processJavaResourcesProvider.configure { dependsOn(syncResources) }

        val stubTask = tasks.getByPath(":app:stub:comment$variantCapped")
        val stubApk = stubTask.outputs.files.asFileTree.filter {
            it.name.endsWith(".apk")
        }

        val syncAssets = tasks.register("sync${variantCapped}Assets", Sync::class) {
            dependsOn(stubTask)
            inputs.property("version", Config.version)
            inputs.property("versionCode", Config.versionCode)
            into("src/${this@all.name}/assets")
            from(rootProject.file("scripts")) {
                include("util_functions.sh", "boot_patch.sh", "addon.d.sh",
                    "app_functions.sh", "uninstaller.sh", "module_installer.sh")
            }
            from(rootProject.file("tools/bootctl"))
            into("chromeos") {
                from(rootProject.file("tools/futility"))
                from(rootProject.file("tools/keys")) {
                    include("kernel_data_key.vbprivk", "kernel.keyblock")
                }
            }
            from(stubApk) {
                rename { "stub.apk" }
            }
            filesMatching("**/util_functions.sh") {
                filter {
                    it.replace(
                        "#MAGISK_VERSION_STUB",
                        "MAGISK_VER='${Config.version}'\nMAGISK_VER_CODE=${Config.versionCode}"
                    )
                }
                filter<FixCrLfFilter>("eol" to FixCrLfFilter.CrLf.newInstance("lf"))
            }
        }
        mergeAssetsProvider.configure { dependsOn(syncAssets) }
    }

    tasks.named<Delete>("clean") {
        delete.addAll(listOf("src/main/jniLibs", "src/main/resources", "src/debug", "src/release"))
    }
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
}

fun Project.setupAppCommon() {
    setupCommon()

    android {
        signingConfigs {
            create("config") {
                Config["keyStore"]?.also {
                    storeFile = rootProject.file(it)
                    storePassword = Config["keyStorePass"]
                    keyAlias = Config["keyAlias"]
                    keyPassword = Config["keyPass"]
                }
            }
        }

        defaultConfig {
            targetSdk = 35
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt")
            )
        }

        buildTypes {
            signingConfigs["config"].also {
                debug {
                    signingConfig = if (it.storeFile?.exists() == true) it
                    else signingConfigs["debug"]
                }
                release {
                    signingConfig = if (it.storeFile?.exists() == true) it
                    else signingConfigs["debug"]
                }
            }
        }

        lint {
            disable += "MissingTranslation"
            checkReleaseBuilds = false
        }

        dependenciesInfo {
            includeInApk = false
        }

        packaging {
            jniLibs {
                useLegacyPackaging = true
            }
        }
    }

    androidComponents.onVariants { variant ->
        val commentTask = tasks.register(
            "comment${variant.name.replaceFirstChar { it.uppercase() }}",
            AddCommentTask::class.java
        )
        val transformationRequest = variant.artifacts.use(commentTask)
            .wiredWithDirectories(AddCommentTask::apkFolder, AddCommentTask::outFolder)
            .toTransformMany(SingleArtifact.APK)
        val signingConfig = androidApp.buildTypes.getByName(variant.buildType!!).signingConfig
        commentTask.configure {
            this.transformationRequest = transformationRequest
            this.signingConfig = signingConfig
            this.comment = "version=${Config.version}\n" +
                "versionCode=${Config.versionCode}\n" +
                "stubVersion=${Config.stubVersion}\n"
            this.outFolder.set(layout.buildDirectory.dir("outputs/apk/${variant.name}"))
        }
    }
}

fun Project.setupMainApk() {
    setupAppCommon()

    android {
        namespace = "com.topjohnwu.magisk"

        defaultConfig {
            applicationId = "com.topjohnwu.magisk"
            vectorDrawables.useSupportLibrary = true
            versionName = Config.version
            versionCode = Config.versionCode
            ndk {
                abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64", "riscv64")
                debugSymbolLevel = "FULL"
            }
        }

        androidComponents.onVariants { variant ->
            variant.instrumentation.apply {
                setAsmFramesComputationMode(COMPUTE_FRAMES_FOR_INSTRUMENTED_METHODS)
                transformClassesWith(
                    DesugarClassVisitorFactory::class.java, InstrumentationScope.ALL) {}
            }
        }
    }
}

fun Project.setupStubApk() {
    setupAppCommon()

    androidComponents.onVariants { variant ->
        val variantName = variant.name
        val variantCapped = variantName.replaceFirstChar { it.uppercase() }
        val manifestUpdater =
            project.tasks.register("${variantName}ManifestProducer", ManifestUpdater::class.java) {
                dependsOn("generate${variantCapped}ObfuscatedClass")
                applicationId = variant.applicationId
                appClassDir.set(layout.buildDirectory.dir("generated/source/app/$variantName"))
                factoryClassDir.set(layout.buildDirectory.dir("generated/source/factory/$variantName"))
            }
        variant.artifacts.use(manifestUpdater)
            .wiredWithFiles(
                ManifestUpdater::mergedManifest,
                ManifestUpdater::outputManifest)
            .toTransform(SingleArtifact.MERGED_MANIFEST)
    }

    androidApp.applicationVariants.all {
        val variantCapped = name.replaceFirstChar { it.uppercase() }
        val variantLowered = name.lowercase()
        val outFactoryClassDir = layout.buildDirectory.file("generated/source/factory/${variantLowered}").get().asFile
        val outAppClassDir = layout.buildDirectory.file("generated/source/app/${variantLowered}").get().asFile
        val outResDir = layout.buildDirectory.dir("generated/source/res/${variantLowered}").get().asFile
        val aapt = File(androidApp.sdkDirectory, "build-tools/${androidApp.buildToolsVersion}/aapt2")
        val apk = layout.buildDirectory.file("intermediates/linked_resources_binary_format/" +
            "${variantLowered}/process${variantCapped}Resources/linked-resources-binary-format-${variantLowered}.ap_").get().asFile

        val genManifestTask = tasks.register("generate${variantCapped}ObfuscatedClass") {
            inputs.property("seed", RAND_SEED)
            outputs.dirs(outFactoryClassDir, outAppClassDir)
            doLast {
                outFactoryClassDir.mkdirs()
                outAppClassDir.mkdirs()
                genStubClasses(outFactoryClassDir, outAppClassDir)
            }
        }
        registerJavaGeneratingTask(genManifestTask, outFactoryClassDir, outAppClassDir)

        val processResourcesTask = tasks.named("process${variantCapped}Resources") {
            outputs.dir(outResDir)
            doLast {
                val apkTmp = File("${apk}.tmp")
                exec {
                    commandLine(aapt, "optimize", "-o", apkTmp, "--collapse-resource-names", apk)
                }

                val bos = ByteArrayOutputStream()
                ZipFile(apkTmp).use { src ->
                    ZipOutputStream(apk.outputStream()).use {
                        it.setLevel(Deflater.BEST_COMPRESSION)
                        it.putNextEntry(ZipEntry("AndroidManifest.xml"))
                        src.getInputStream(src.getEntry("AndroidManifest.xml")).transferTo(it)
                        it.closeEntry()
                    }
                    DeflaterOutputStream(bos, Deflater(Deflater.BEST_COMPRESSION)).use {
                        src.getInputStream(src.getEntry("resources.arsc")).transferTo(it)
                    }
                }
                apkTmp.delete()
                genEncryptedResources(bos.toByteArray(), outResDir)
            }
        }

        registerJavaGeneratingTask(processResourcesTask, outResDir)
    }
    // Override optimizeReleaseResources task
    val apk = layout.buildDirectory.file("intermediates/linked_resources_binary_format/" +
        "release/processReleaseResources/linked-resources-binary-format-release.ap_").get().asFile
    val optRes = layout.buildDirectory.file("intermediates/optimized_processed_res/" +
        "release/optimizeReleaseResources/resources-release-optimize.ap_").get().asFile
    afterEvaluate {
        tasks.named("optimizeReleaseResources") {
            doLast { apk.copyTo(optRes, true) }
        }
    }
    tasks.named<Delete>("clean") {
        delete.addAll(listOf("src/debug/AndroidManifest.xml", "src/release/AndroidManifest.xml"))
    }
}

const val LSPOSED_DOWNLOAD_URL =
    "https://github.com/LSPosed/LSPosed/releases/download/v1.9.2/LSPosed-v1.9.2-7024-zygisk-release.zip"
const val LSPOSED_CHECKSUM =
    "0ebc6bcb465d1c4b44b7220ab5f0252e6b4eb7fe43da74650476d2798bb29622"

const val SHAMIKO_DOWNLOAD_URL =
    "https://github.com/LSPosed/LSPosed.github.io/releases/download/shamiko-383/Shamiko-v1.2.1-383-release.zip"
const val SHAMIKO_CHECKSUM =
    "93754a038c2d8f0e985bad45c7303b96f70a93d8335060e50146f028d3a9b13f"

fun Project.setupTestApk() {
    setupAppCommon()

    androidApp.applicationVariants.all {
        val variantCapped = name.replaceFirstChar { it.uppercase() }
        val dlTask by tasks.register("download${variantCapped}Lsposed", Sync::class) {
            from(downloadFile(LSPOSED_DOWNLOAD_URL, LSPOSED_CHECKSUM)) {
                rename { "lsposed.zip" }
            }
            from(downloadFile(SHAMIKO_DOWNLOAD_URL, SHAMIKO_CHECKSUM)) {
                rename { "shamiko.zip" }
            }
            into("src/${this@all.name}/assets")
        }
        mergeAssetsProvider.configure { dependsOn(dlTask) }
    }
}
