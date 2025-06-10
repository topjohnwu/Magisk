import com.android.build.api.artifact.SingleArtifact
import com.android.build.api.instrumentation.FramesComputationMode.COMPUTE_FRAMES_FOR_INSTRUMENTED_METHODS
import com.android.build.api.instrumentation.InstrumentationScope
import com.android.build.api.variant.ApplicationAndroidComponentsExtension
import com.android.build.gradle.BaseExtension
import com.android.build.gradle.LibraryExtension
import com.android.build.gradle.internal.dsl.BaseAppModuleExtension
import org.apache.tools.ant.filters.FixCrLfFilter
import org.gradle.api.Action
import org.gradle.api.JavaVersion
import org.gradle.api.Project
import org.gradle.api.tasks.Copy
import org.gradle.api.tasks.Delete
import org.gradle.api.tasks.StopExecutionException
import org.gradle.api.tasks.Sync
import org.gradle.kotlin.dsl.assign
import org.gradle.kotlin.dsl.exclude
import org.gradle.kotlin.dsl.filter
import org.gradle.kotlin.dsl.get
import org.gradle.kotlin.dsl.getValue
import org.gradle.kotlin.dsl.named
import org.gradle.kotlin.dsl.provideDelegate
import org.gradle.kotlin.dsl.register
import org.gradle.kotlin.dsl.withType
import org.jetbrains.kotlin.gradle.dsl.JvmTarget
import org.jetbrains.kotlin.gradle.tasks.KotlinCompile
import java.io.ByteArrayOutputStream
import java.io.File
import java.net.URI
import java.security.MessageDigest
import java.util.HexFormat
import java.util.zip.Deflater
import java.util.zip.DeflaterOutputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

private fun Project.androidBase(configure: Action<BaseExtension>) =
    extensions.configure("android", configure)

private fun Project.android(configure: Action<BaseAppModuleExtension>) =
    extensions.configure("android", configure)

internal val Project.androidApp: BaseAppModuleExtension
    get() = extensions["android"] as BaseAppModuleExtension

private val Project.androidLib: LibraryExtension
    get() = extensions["android"] as LibraryExtension

internal val Project.androidComponents
    get() = extensions.getByType(ApplicationAndroidComponentsExtension::class.java)

fun Project.setupCommon() {
    androidBase {
        compileSdkVersion(36)
        buildToolsVersion = "36.0.0"
        ndkPath = "$sdkDirectory/ndk/magisk"
        ndkVersion = "28.1.13356709"

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
                    "/META-INF/androidx/**",
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

    androidLib.libraryVariants.all {
        val variant = name
        val variantCapped = name.replaceFirstChar { it.uppercase() }
        val abiList = Config.abiList

        val syncLibs = tasks.register("sync${variantCapped}JniLibs", Sync::class) {
            into("src/$variant/jniLibs")
            for (abi in abiList) {
                into(abi) {
                    from(rootFile("native/out/$abi")) {
                        include("magiskboot", "magiskinit", "magiskpolicy", "magisk", "libinit-ld.so")
                        rename { if (it.endsWith(".so")) it else "lib$it.so" }
                    }
                }
            }
            from(zipTree(downloadFile(BUSYBOX_DOWNLOAD_URL, BUSYBOX_ZIP_CHECKSUM)))
            include(abiList.map { "$it/libbusybox.so" })
            onlyIf {
                if (inputs.sourceFiles.files.size != abiList.size * 6)
                    throw StopExecutionException("Please build binaries first! (./build.py binary)")
                true
            }
        }

        tasks.getByPath("merge${variantCapped}JniLibFolders").dependsOn(syncLibs)

        val syncResources = tasks.register("sync${variantCapped}Resources", Sync::class) {
            into("src/$variant/resources/META-INF/com/google/android")
            from(rootFile("scripts/update_binary.sh")) {
                rename { "update-binary" }
            }
            from(rootFile("scripts/flash_script.sh")) {
                rename { "updater-script" }
            }
        }

        processJavaResourcesProvider.configure { dependsOn(syncResources) }

        val stubTask = tasks.getByPath(":stub:comment$variantCapped")
        val stubApk = stubTask.outputs.files.asFileTree.filter {
            it.name.endsWith(".apk")
        }

        val syncAssets = tasks.register("sync${variantCapped}Assets", Sync::class) {
            dependsOn(stubTask)
            inputs.property("version", Config.version)
            inputs.property("versionCode", Config.versionCode)
            into("src/$variant/assets")
            from(rootFile("scripts")) {
                include("util_functions.sh", "boot_patch.sh", "addon.d.sh",
                    "app_functions.sh", "uninstaller.sh", "module_installer.sh")
            }
            from(rootFile("tools/bootctl"))
            into("chromeos") {
                from(rootFile("tools/futility"))
                from(rootFile("tools/keys")) {
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

fun Project.setupAppCommon() {
    setupCommon()

    android {
        signingConfigs {
            Config["keyStore"]?.also {
                create("config") {
                    storeFile = rootFile(it)
                    storePassword = Config["keyStorePass"]
                    keyAlias = Config["keyAlias"]
                    keyPassword = Config["keyPass"]
                }
            }
        }

        defaultConfig {
            targetSdk = 36
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt")
            )
        }

        buildTypes {
            val config = signingConfigs.findByName("config") ?: signingConfigs["debug"]
            debug {
                signingConfig = config
            }
            release {
                signingConfig = config
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
