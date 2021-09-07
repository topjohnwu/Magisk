import com.android.build.gradle.BaseExtension
import org.gradle.api.Action
import org.gradle.api.JavaVersion
import org.gradle.api.Project
import org.gradle.api.tasks.compile.JavaCompile
import org.gradle.kotlin.dsl.withType

private fun Project.android(configure: Action<BaseExtension>) = extensions.configure("android", configure)

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

fun Project.setupApp() {
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
