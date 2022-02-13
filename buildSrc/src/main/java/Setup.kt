import com.android.build.gradle.BaseExtension
import com.android.build.gradle.internal.dsl.BaseAppModuleExtension
import org.apache.tools.ant.filters.FixCrLfFilter
import org.gradle.api.Action
import org.gradle.api.JavaVersion
import org.gradle.api.Project
import org.gradle.api.tasks.Delete
import org.gradle.api.tasks.StopExecutionException
import org.gradle.api.tasks.Sync
import org.gradle.kotlin.dsl.filter
import org.gradle.kotlin.dsl.named
import java.io.*
import java.util.*
import java.util.zip.Deflater
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

private fun Project.androidBase(configure: Action<BaseExtension>) =
    extensions.configure("android", configure)

private fun Project.android(configure: Action<BaseAppModuleExtension>) =
    extensions.configure("android", configure)

private val Project.android: BaseAppModuleExtension
    get() = extensions.getByName("android") as BaseAppModuleExtension

fun Project.setupCommon() {
    androidBase {
        compileSdkVersion(31)
        buildToolsVersion = "31.0.0"
        ndkPath = "$sdkDirectory/ndk/magisk"

        defaultConfig {
            minSdk = 21
            targetSdk = 31
        }

        compileOptions {
            sourceCompatibility = JavaVersion.VERSION_11
            targetCompatibility = JavaVersion.VERSION_11
        }
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
            signingConfigs.getByName("config").also {
                getByName("debug") {
                    signingConfig = if (it.storeFile?.exists() == true) it
                    else signingConfigs.getByName("debug")
                }
                getByName("release") {
                    signingConfig = if (it.storeFile?.exists() == true) it
                    else signingConfigs.getByName("debug")
                }
            }
        }

        lint {
            disable += "MissingTranslation"
        }

        dependenciesInfo {
            includeInApk = false
        }
    }
}

fun Project.setupApp() {
    setupAppCommon()

    val syncLibs = tasks.register("syncLibs", Sync::class.java) {
        into("src/main/jniLibs")
        into("armeabi-v7a") {
            from(rootProject.file("native/out/armeabi-v7a")) {
                include("busybox", "magiskboot", "magiskinit", "magisk")
                rename { if (it == "magisk") "libmagisk32.so" else "lib$it.so" }
            }
        }
        into("x86") {
            from(rootProject.file("native/out/x86")) {
                include("busybox", "magiskboot", "magiskinit", "magisk")
                rename { if (it == "magisk") "libmagisk32.so" else "lib$it.so" }
            }
        }
        into("arm64-v8a") {
            from(rootProject.file("native/out/arm64-v8a")) {
                include("busybox", "magiskboot", "magiskinit", "magisk")
                rename { if (it == "magisk") "libmagisk64.so" else "lib$it.so" }
            }
        }
        into("x86_64") {
            from(rootProject.file("native/out/x86_64")) {
                include("busybox", "magiskboot", "magiskinit", "magisk")
                rename { if (it == "magisk") "libmagisk64.so" else "lib$it.so" }
            }
        }
        onlyIf {
            if (inputs.sourceFiles.files.size != 16)
                throw StopExecutionException("Please build binaries first! (./build.py binary)")
            true
        }
    }

    val syncAssets = tasks.register("syncAssets", Sync::class.java) {
        dependsOn(syncLibs)
        inputs.property("version", Config.version)
        inputs.property("versionCode", Config.versionCode)
        into("src/main/assets")
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

    val syncResources = tasks.register("syncResources", Sync::class.java) {
        dependsOn(syncAssets)
        into("src/main/resources/META-INF/com/google/android")
        from(rootProject.file("scripts/update_binary.sh")) {
            rename { "update-binary" }
        }
        from(rootProject.file("scripts/flash_script.sh")) {
            rename { "updater-script" }
        }
    }

    android.applicationVariants.all {
        preBuildProvider.get().dependsOn(syncResources)

        val keysDir = rootProject.file("tools/keys")
        val outSrcDir = File(buildDir, "generated/source/keydata/$name")
        val outSrc = File(outSrcDir, "com/topjohnwu/magisk/signing/KeyData.java")

        val genSrcTask = tasks.register("generate${name.capitalize(Locale.ROOT)}KeyData") {
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
        val variantCapped = name.capitalize(Locale.ROOT)
        val variantLowered = name.toLowerCase(Locale.ROOT)
        val manifest = file("src/${variantLowered}/AndroidManifest.xml")
        val outSrcDir = File(buildDir, "generated/source/obfuscate/${variantLowered}")
        val templateDir = file("template")
        val aapt = File(android.sdkDirectory, "build-tools/${android.buildToolsVersion}/aapt2")
        val apk = File(buildDir, "intermediates/processed_res/" +
            "${variantLowered}/out/resources-${variantLowered}.ap_")
        val apkTmp = File("${apk}.tmp")

        val genManifestTask = tasks.register("generate${variantCapped}ObfuscatedManifest") {
            doLast {
                val xml = genStubManifest(templateDir, outSrcDir)
                manifest.parentFile.mkdirs()
                PrintStream(manifest).use {
                    it.print(xml)
                }
            }
        }
        tasks.getByPath(":stub:process${variantCapped}MainManifest").dependsOn(genManifestTask)

        val genSrcTask = tasks.register("generate${variantCapped}ObfuscatedSources") {
            dependsOn(":stub:process${variantCapped}Resources")
            inputs.file(apk)
            outputs.file(apk)
            doLast {
                exec {
                    commandLine(aapt, "optimize", "-o", apkTmp, "--collapse-resource-names", apk)
                }

                val buffer = ByteArrayOutputStream(apk.length().toInt())
                val newApk = ZipOutputStream(FileOutputStream(apk))
                ZipFile(apkTmp).use {
                    newApk.use { new ->
                        new.setLevel(Deflater.BEST_COMPRESSION)
                        new.putNextEntry(ZipEntry("AndroidManifest.xml"))
                        it.getInputStream(it.getEntry("AndroidManifest.xml")).transferTo(new)
                        new.closeEntry()
                        new.finish()
                    }
                    ZipOutputStream(buffer).use { arsc ->
                        arsc.setLevel(Deflater.BEST_COMPRESSION)
                        arsc.putNextEntry(ZipEntry("resources.arsc"))
                        it.getInputStream(it.getEntry("resources.arsc")).transferTo(arsc)
                        arsc.closeEntry()
                        arsc.finish()
                    }
                }
                apkTmp.delete()
                genEncryptedResources(ByteArrayInputStream(buffer.toByteArray()), outSrcDir)
            }
        }
        registerJavaGeneratingTask(genSrcTask, outSrcDir)
    }
    // Override optimizeReleaseResources task
    tasks.whenTaskAdded {
        val apk = File(buildDir, "intermediates/processed_res/" +
            "release/out/resources-release.ap_")
        val optRes = File(buildDir, "intermediates/optimized_processed_res/" +
            "release/resources-release-optimize.ap_")
        if (name == "optimizeReleaseResources") {
            doLast { apk.copyTo(optRes, true) }
        }
    }
    tasks.named<Delete>("clean") {
        delete.addAll(listOf("src/debug/AndroidManifest.xml", "src/release/AndroidManifest.xml"))
    }
}
