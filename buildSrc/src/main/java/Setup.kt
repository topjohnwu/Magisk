
import com.android.build.gradle.BaseExtension
import com.android.build.gradle.internal.dsl.BaseAppModuleExtension
import org.apache.tools.ant.filters.FixCrLfFilter
import org.gradle.api.Action
import org.gradle.api.DefaultTask
import org.gradle.api.JavaVersion
import org.gradle.api.Project
import org.gradle.api.tasks.StopExecutionException
import org.gradle.api.tasks.Sync
import org.gradle.api.tasks.compile.JavaCompile
import org.gradle.internal.os.OperatingSystem
import org.gradle.kotlin.dsl.filter
import org.gradle.kotlin.dsl.named
import org.gradle.kotlin.dsl.withType
import java.io.File
import java.io.OutputStream
import java.io.PrintStream
import java.nio.file.Paths
import java.util.*

private fun Project.android(configure: Action<BaseExtension>) =
    extensions.configure("android", configure)

private val Project.android: BaseAppModuleExtension get() =
    extensions.getByName("android") as BaseAppModuleExtension

fun Project.setupCommon() {
    android {
        compileSdkVersion(31)
        buildToolsVersion = "31.0.0"
        ndkPath = "${System.getenv("ANDROID_SDK_ROOT")}/ndk/magisk"

        defaultConfig {
            minSdk = 21
            targetSdk = 31
        }

        compileOptions {
            sourceCompatibility = JavaVersion.VERSION_11
            targetCompatibility = JavaVersion.VERSION_11
        }
    }

    if (plugins.hasPlugin("java")) {
        tasks.withType<JavaCompile> {
            // If building with JDK 9+, we need additional flags to generate compatible bytecode
            if (JavaVersion.current() > JavaVersion.VERSION_1_8) {
                options.compilerArgs.addAll(listOf("--release", "8"))
            }
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

        lintOptions {
            disable += "MissingTranslation"
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
            include("util_functions.sh", "boot_patch.sh", "uninstaller.sh", "addon.d.sh")
        }
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

    tasks.named<DefaultTask>("preBuild") {
        dependsOn(syncResources)
    }

    android.applicationVariants.all {
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

    // Make sure we have a working manifest while building
    val ensureManifest = tasks.register("ensureManifest") {
        val manifest = file("src/main/AndroidManifest.xml")
        if (!manifest.exists()) {
            PrintStream(manifest).use {
                it.println("<manifest package=\"com.topjohnwu.magisk\"/>")
            }
        }
    }

    tasks.named<DefaultTask>("preBuild") {
        dependsOn(ensureManifest)
    }

    android.applicationVariants.all {
        val manifest = file("src/main/AndroidManifest.xml")
        val outSrcDir = File(buildDir, "generated/source/obfuscate/$name")
        val templateDir = file("template")
        val resDir = file("res")

        val androidJar = Paths.get(android.sdkDirectory.path, "platforms",
            android.compileSdkVersion, "android.jar")

        val aaptCommand = if (OperatingSystem.current().isWindows) "aapt2.exe" else "aapt2"
        val aapt = Paths.get(android.sdkDirectory.path,
            "build-tools", android.buildToolsVersion, aaptCommand)

        val dummy = object : OutputStream() {
            override fun write(b: Int) {}
            override fun write(bytes: ByteArray, off: Int, len: Int) {}
        }

        val genSrcTask = tasks.register("generate${name.capitalize(Locale.ROOT)}ObfuscatedSources") {
            doLast {
                val xml = genStubManifest(templateDir, outSrcDir)
                PrintStream(manifest).use {
                    it.print(xml)
                }

                val compileTmp = File.createTempFile("tmp", ".zip")
                val linkTmp = File.createTempFile("tmp", ".zip")
                val optTmp = File.createTempFile("tmp", ".zip")
                val stubXml = File.createTempFile("tmp", ".xml")
                try {
                    PrintStream(stubXml).use {
                        it.println("<manifest package=\"com.topjohnwu.magisk\"/>")
                    }

                    exec {
                        commandLine(aapt, "compile",
                            "-o", compileTmp,
                            "--dir", resDir)
                        standardOutput = dummy
                        errorOutput = dummy
                    }

                    exec {
                        commandLine(aapt, "link",
                            "-o", linkTmp,
                            "-I", androidJar,
                            "--min-sdk-version", android.defaultConfig.minSdk,
                            "--target-sdk-version", android.defaultConfig.targetSdk,
                            "--manifest", stubXml,
                            "--java", outSrcDir, compileTmp)
                        standardOutput = dummy
                        errorOutput = dummy
                    }

                    exec {
                        commandLine(aapt, "optimize",
                            "-o", optTmp,
                            "--collapse-resource-names", linkTmp)
                        standardOutput = dummy
                        errorOutput = dummy
                    }

                    genEncryptedResources(optTmp, outSrcDir)
                } finally {
                    compileTmp.delete()
                    linkTmp.delete()
                    optTmp.delete()
                    stubXml.delete()
                }
            }
        }
        registerJavaGeneratingTask(genSrcTask, outSrcDir)
    }
}
