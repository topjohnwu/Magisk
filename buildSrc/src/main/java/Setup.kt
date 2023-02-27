import com.android.build.gradle.BaseExtension
import com.android.build.gradle.internal.dsl.BaseAppModuleExtension
import com.android.builder.internal.packaging.IncrementalPackager
import com.android.builder.model.SigningConfig
import com.android.tools.build.apkzlib.sign.SigningExtension
import com.android.tools.build.apkzlib.sign.SigningOptions
import com.android.tools.build.apkzlib.zfile.ZFiles
import com.android.tools.build.apkzlib.zip.ZFileOptions
import org.apache.tools.ant.filters.FixCrLfFilter
import org.gradle.api.Action
import org.gradle.api.JavaVersion
import org.gradle.api.Project
import org.gradle.api.plugins.ExtensionAware
import org.gradle.api.tasks.Delete
import org.gradle.api.tasks.StopExecutionException
import org.gradle.api.tasks.Sync
import org.gradle.kotlin.dsl.*
import org.jetbrains.kotlin.gradle.dsl.KotlinAndroidProjectExtension
import org.jetbrains.kotlin.gradle.dsl.KotlinJvmOptions
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.PrintStream
import java.security.KeyStore
import java.security.cert.X509Certificate
import java.util.*
import java.util.jar.JarFile
import java.util.zip.*

private fun Project.androidBase(configure: Action<BaseExtension>) =
    extensions.configure("android", configure)

private fun Project.android(configure: Action<BaseAppModuleExtension>) =
    extensions.configure("android", configure)

private fun BaseExtension.kotlinOptions(configure: Action<KotlinJvmOptions>) =
    (this as ExtensionAware).extensions.findByName("kotlinOptions")?.let {
        configure.execute(it as KotlinJvmOptions)
    }

private fun BaseExtension.kotlin(configure: Action<KotlinAndroidProjectExtension>) =
    (this as ExtensionAware).extensions.findByName("kotlin")?.let {
        configure.execute(it as KotlinAndroidProjectExtension)
    }

private val Project.android: BaseAppModuleExtension
    get() = extensions["android"] as BaseAppModuleExtension

fun Project.setupCommon() {
    androidBase {
        compileSdkVersion(33)
        buildToolsVersion = "33.0.1"
        ndkPath = "$sdkDirectory/ndk/magisk"

        defaultConfig {
            minSdk = 21
            targetSdk = 33
        }

        compileOptions {
            sourceCompatibility = JavaVersion.VERSION_17
            targetCompatibility = JavaVersion.VERSION_17
        }

        kotlinOptions {
            jvmTarget = "17"
        }

        kotlin {
            jvmToolchain(17)
        }
    }
}

private fun SigningConfig.getPrivateKey(): KeyStore.PrivateKeyEntry {
    val keyStore = KeyStore.getInstance(storeType ?: KeyStore.getDefaultType())
    storeFile!!.inputStream().use {
        keyStore.load(it, storePassword!!.toCharArray())
    }
    val keyPwdArray = keyPassword!!.toCharArray()
    val entry = keyStore.getEntry(keyAlias!!, KeyStore.PasswordProtection(keyPwdArray))
    return entry as KeyStore.PrivateKeyEntry
}

private fun addComment(apkPath: File, signConfig: SigningConfig, minSdk: Int, eocdComment: String) {
    val privateKey = signConfig.getPrivateKey()
    val signingOptions = SigningOptions.builder()
        .setMinSdkVersion(minSdk)
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
    ZFiles.apk(apkPath, options).use {
        SigningExtension(signingOptions).register(it)
        it.eocdComment = eocdComment.toByteArray()
        it.get(IncrementalPackager.APP_METADATA_ENTRY_PATH)?.delete()
        it.get(JarFile.MANIFEST_NAME)?.delete()
    }
}

private fun Project.setupAppCommon() {
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

        buildFeatures {
            buildConfig = true
        }
    }

    android.applicationVariants.all {
        val projectName = project.name.lowercase()
        val variantCapped = name.replaceFirstChar { it.uppercase() }
        tasks.getByPath(":$projectName:package$variantCapped").doLast {
            val apk = outputs.files.asFileTree.filter { it.name.endsWith(".apk") }.singleFile
            val comment = "version=${Config.version}\n" +
                "versionCode=${Config.versionCode}\n" +
                "stubVersion=${Config.stubVersion}}\n"
            addComment(apk, signingConfig, android.defaultConfig.minSdk!!, comment)
        }
    }
}

fun Project.setupApp() {
    setupAppCommon()

    val syncLibs by tasks.registering(Sync::class) {
        into("src/main/jniLibs")
        into("armeabi-v7a") {
            from(rootProject.file("native/out/armeabi-v7a")) {
                include("busybox", "magiskboot", "magiskinit", "magiskpolicy", "magisk")
                rename { if (it == "magisk") "libmagisk32.so" else "lib$it.so" }
            }
        }
        into("x86") {
            from(rootProject.file("native/out/x86")) {
                include("busybox", "magiskboot", "magiskinit", "magiskpolicy", "magisk")
                rename { if (it == "magisk") "libmagisk32.so" else "lib$it.so" }
            }
        }
        into("arm64-v8a") {
            from(rootProject.file("native/out/arm64-v8a")) {
                include("busybox", "magiskboot", "magiskinit", "magiskpolicy", "magisk")
                rename { if (it == "magisk") "libmagisk64.so" else "lib$it.so" }
            }
        }
        into("x86_64") {
            from(rootProject.file("native/out/x86_64")) {
                include("busybox", "magiskboot", "magiskinit", "magiskpolicy", "magisk")
                rename { if (it == "magisk") "libmagisk64.so" else "lib$it.so" }
            }
        }
        onlyIf {
            if (inputs.sourceFiles.files.size != 20)
                throw StopExecutionException("Please build binaries first! (./build.py binary)")
            true
        }
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

    android.applicationVariants.all {
        val variantCapped = name.replaceFirstChar { it.uppercase() }

        tasks.getByPath("merge${variantCapped}JniLibFolders").dependsOn(syncLibs)
        processJavaResourcesProvider.configure { dependsOn(syncResources) }

        val stubTask = tasks.getByPath(":stub:package$variantCapped")
        val stubApk = stubTask.outputs.files.asFileTree.filter {
            it.name.endsWith(".apk")
        }

        val syncAssets = tasks.register("sync${variantCapped}Assets", Sync::class) {
            dependsOn(stubTask)
            inputs.property("version", Config.version)
            inputs.property("versionCode", Config.versionCode)
            into("src/${this@all.name}/assets")
            from(rootProject.file("scripts")) {
                include("util_functions.sh", "boot_patch.sh", "addon.d.sh")
                include("uninstaller.sh", "module_installer.sh")
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

        val keysDir = rootProject.file("tools/keys")
        val outSrcDir = File(buildDir, "generated/source/keydata/$name")
        val outSrc = File(outSrcDir, "com/topjohnwu/magisk/signing/KeyData.java")

        val genSrcTask = tasks.register("generate${variantCapped}KeyData") {
            inputs.dir(keysDir)
            outputs.file(outSrc)
            doLast {
                genKeyData(keysDir, outSrc)
            }
        }
        registerJavaGeneratingTask(genSrcTask, outSrcDir)
    }
}

fun Project.setupStub() {
    setupAppCommon()

    android.applicationVariants.all {
        val variantCapped = name.replaceFirstChar { it.uppercase() }
        val variantLowered = name.lowercase()
        val manifest = file("src/${variantLowered}/AndroidManifest.xml")
        val outManifestDir = File(buildDir, "generated/source/manifest/${variantLowered}")
        val outResDir = File(buildDir, "generated/source/res/${variantLowered}")
        val templateDir = file("template")
        val aapt = File(android.sdkDirectory, "build-tools/${android.buildToolsVersion}/aapt2")
        val apk = File(buildDir, "intermediates/processed_res/" +
            "${variantLowered}/out/resources-${variantLowered}.ap_")
        val apkTmp = File("${apk}.tmp")

        val genManifestTask = tasks.register("generate${variantCapped}ObfuscatedManifest") {
            inputs.property("seed", RAND_SEED)
            inputs.dir(templateDir)
            outputs.dir(outManifestDir)
            outputs.file(manifest)
            doLast {
                val xml = genStubManifest(templateDir, outManifestDir)
                manifest.parentFile.mkdirs()
                PrintStream(manifest).use {
                    it.print(xml)
                }
            }
        }
        preBuildProvider.configure { dependsOn(genManifestTask) }
        registerJavaGeneratingTask(genManifestTask, outManifestDir)

        val processResourcesTask = tasks.getByPath(":stub:process${variantCapped}Resources")
        processResourcesTask.doLast {
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
            genEncryptedResources(ByteArrayInputStream(bos.toByteArray()), outResDir)
        }
        @Suppress("DEPRECATION")
        registerJavaGeneratingTask(processResourcesTask, outResDir)
    }
    // Override optimizeReleaseResources task
    val apk = File(buildDir, "intermediates/processed_res/" +
        "release/out/resources-release.ap_")
    val optRes = File(buildDir, "intermediates/optimized_processed_res/" +
        "release/resources-release-optimize.ap_")
    tasks.whenTaskAdded {
        if (name == "optimizeReleaseResources") {
            doLast { apk.copyTo(optRes, true) }
        }
    }
    tasks.named<Delete>("clean") {
        delete.addAll(listOf("src/debug/AndroidManifest.xml", "src/release/AndroidManifest.xml"))
    }
}
