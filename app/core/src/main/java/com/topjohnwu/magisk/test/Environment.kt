package com.topjohnwu.magisk.test

import android.app.Notification
import android.content.Context
import android.os.Build
import androidx.annotation.Keep
import androidx.core.net.toUri
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.topjohnwu.magisk.core.BuildConfig.APP_PACKAGE_NAME
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.di.ServiceLocator
import com.topjohnwu.magisk.core.download.DownloadNotifier
import com.topjohnwu.magisk.core.download.DownloadProcessor
import com.topjohnwu.magisk.core.ktx.cachedFile
import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.core.tasks.AppMigration
import com.topjohnwu.magisk.core.tasks.FlashZip
import com.topjohnwu.magisk.core.tasks.MagiskInstaller
import com.topjohnwu.superuser.CallbackList
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertTrue
import org.junit.Assume.assumeTrue
import org.junit.Before
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import timber.log.Timber

@Keep
@RunWith(AndroidJUnit4::class)
class Environment {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() = MagiskAppTest.before()

        fun lsposed(): Boolean {
            return Build.VERSION.SDK_INT >= 27 && Build.VERSION.SDK_INT <= 34
        }

        private const val LSPOSED_URL =
            "https://github.com/LSPosed/LSPosed/releases/download/v1.9.2/LSPosed-v1.9.2-7024-zygisk-release.zip"
    }

    object TimberLog : CallbackList<String>(Runnable::run) {
        override fun onAddElement(e: String) {
            Timber.i(e)
        }
    }

    private lateinit var mContext: Context

    @Before
    fun setup() {
        mContext = InstrumentationRegistry.getInstrumentation().targetContext
    }

    @Test
    fun setupMagisk() {
        runBlocking {
            assertTrue(
                "Magisk setup failed",
                MagiskInstaller.Emulator(TimberLog, TimberLog).exec()
            )
        }
    }

    @Test
    fun setupLsposed() {
        assumeTrue(lsposed())

        val notify = object : DownloadNotifier {
            override val context = mContext
            override fun notifyUpdate(id: Int, editor: (Notification.Builder) -> Unit) {}
        }
        val processor = DownloadProcessor(notify)
        val zip = mContext.cachedFile("lsposed.zip")
        runBlocking {
            ServiceLocator.networkService.fetchFile(LSPOSED_URL).byteStream().use {
                processor.handleModule(it, zip.toUri())
            }
            assertTrue(
                "LSPosed installation failed",
                FlashZip(zip.toUri(), TimberLog, TimberLog).exec()
            )
        }
    }

    @Test
    fun setupShellGrantTest() {
        runBlocking {
            // Inject an undetermined + mute logging policy for ADB shell
            val policy = SuPolicy(
                uid = 2000,
                logging = false,
                notification = false,
                until = 0L
            )
            ServiceLocator.policyDB.update(policy)
            // Bypass the need to actually show a dialog
            Config.suAutoResponse = Config.Value.SU_AUTO_ALLOW
            Config.prefs.edit().commit()
        }
    }

    @Test
    fun setupAppHide() {
        runBlocking {
            assertTrue(
                "App hiding failed",
                AppMigration.patchAndHide(
                    context = mContext,
                    label = "Settings",
                    pkg = "repackaged.$APP_PACKAGE_NAME"
                )
            )
        }
    }

    @Test
    fun setupAppRestore() {
        runBlocking {
            assertTrue(
                "App restoration failed",
                AppMigration.restoreApp(mContext)
            )
        }
    }
}
