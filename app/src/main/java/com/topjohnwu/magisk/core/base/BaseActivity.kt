package com.topjohnwu.magisk.core.base

import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Parcelable
import android.widget.Toast
import androidx.activity.result.ActivityResultCallback
import androidx.activity.result.contract.ActivityResultContracts.GetContent
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.annotation.WorkerThread
import androidx.appcompat.app.AppCompatActivity
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.utils.RequestInstall
import com.topjohnwu.magisk.core.utils.UninstallPackage
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ktx.reflectField
import com.topjohnwu.magisk.utils.Utils
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

interface ContentResultCallback: ActivityResultCallback<Uri>, Parcelable {
    fun onActivityLaunch() {}
    // Make the result type explicitly non-null
    override fun onActivityResult(result: Uri)
}

abstract class BaseActivity : AppCompatActivity() {

    private var permissionCallback: ((Boolean) -> Unit)? = null
    private val requestPermission = registerForActivityResult(RequestPermission()) {
        permissionCallback?.invoke(it)
        permissionCallback = null
    }
    private val requestInstall = registerForActivityResult(RequestInstall()) {
        permissionCallback?.invoke(it)
        permissionCallback = null
    }

    private var contentCallback: ContentResultCallback? = null
    private val getContent = registerForActivityResult(GetContent()) {
        if (it != null) contentCallback?.onActivityResult(it)
        contentCallback = null
    }

    private var uninstallLatch = CountDownLatch(1)
    private val uninstallPkg = registerForActivityResult(UninstallPackage()) {
        uninstallLatch.countDown()
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        if (isRunningAsStub) {
            // Overwrite private members to avoid nasty "false" stack traces being logged
            val delegate = delegate
            val clz = delegate.javaClass
            clz.reflectField("mActivityHandlesUiModeChecked").set(delegate, true)
            clz.reflectField("mActivityHandlesUiMode").set(delegate, false)
        }
        contentCallback = savedInstanceState?.getParcelable(CONTENT_CALLBACK_KEY)
        super.onCreate(savedInstanceState)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        contentCallback?.let {
            outState.putParcelable(CONTENT_CALLBACK_KEY, it)
        }
    }

    fun withPermission(permission: String, callback: (Boolean) -> Unit) {
        if (permission == WRITE_EXTERNAL_STORAGE && Build.VERSION.SDK_INT >= 30) {
            // We do not need external rw on 30+
            callback(true)
            return
        }
        permissionCallback = callback
        if (permission == REQUEST_INSTALL_PACKAGES) {
            requestInstall.launch(Unit)
        } else {
            requestPermission.launch(permission)
        }
    }

    fun getContent(type: String, callback: ContentResultCallback) {
        contentCallback = callback
        try {
            getContent.launch(type)
            callback.onActivityLaunch()
        } catch (e: ActivityNotFoundException) {
            Utils.toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }

    @WorkerThread
    fun uninstallAndWait(pkg: String) {
        uninstallLatch = CountDownLatch(1)
        uninstallPkg.launch(pkg)
        uninstallLatch.await(3, TimeUnit.SECONDS)
    }

    override fun recreate() {
        startActivity(Intent().setComponent(intent.component))
        finish()
    }

    fun relaunch() {
        startActivity(Intent(intent).setFlags(0))
        finish()
    }

    companion object {
        private const val CONTENT_CALLBACK_KEY = "content_callback"
    }
}
