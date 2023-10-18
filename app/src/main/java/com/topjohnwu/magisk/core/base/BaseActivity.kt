package com.topjohnwu.magisk.core.base

import android.Manifest.permission.POST_NOTIFICATIONS
import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.app.Activity
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
import androidx.appcompat.app.AppCompatActivity
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.reflectField
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.utils.RequestAuthentication
import com.topjohnwu.magisk.core.utils.RequestInstall
import com.topjohnwu.magisk.core.wrap

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

    private var installCallback: ((Boolean) -> Unit)? = null
    private val requestInstall = registerForActivityResult(RequestInstall()) {
        installCallback?.invoke(it)
        installCallback = null
    }

    var authenticateCallback: ((Boolean) -> Unit)? = null
    val requestAuthenticate = registerForActivityResult(RequestAuthentication()) {
        authenticateCallback?.invoke(it)
        authenticateCallback = null
    }

    private var contentCallback: ContentResultCallback? = null
    private val getContent = registerForActivityResult(GetContent()) {
        if (it != null) contentCallback?.onActivityResult(it)
        contentCallback = null
    }

    private val mReferrerField by lazy(LazyThreadSafetyMode.NONE) {
        Activity::class.java.reflectField("mReferrer")
    }

    val realCallingPackage: String? get() {
        callingPackage?.let { return it }
        mReferrerField.get(this)?.let { return it as String }
        return null
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        if (isRunningAsStub) {
            // Overwrite private members to avoid nasty "false" stack traces being logged
            val delegate = delegate
            val clz = delegate.javaClass
            clz.reflectField("mActivityHandlesConfigFlagsChecked").set(delegate, true)
            clz.reflectField("mActivityHandlesConfigFlags").set(delegate, 0)
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
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R &&
            permission == WRITE_EXTERNAL_STORAGE) {
            // We do not need external rw on R+
            callback(true)
            return
        }
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU &&
            permission == POST_NOTIFICATIONS) {
            // All apps have notification permissions before T
            callback(true)
            return
        }
        if (permission == REQUEST_INSTALL_PACKAGES) {
            installCallback = callback
            requestInstall.launch(Unit)
        } else {
            permissionCallback = callback
            requestPermission.launch(permission)
        }
    }

    fun getContent(type: String, callback: ContentResultCallback) {
        contentCallback = callback
        try {
            getContent.launch(type)
            callback.onActivityLaunch()
        } catch (e: ActivityNotFoundException) {
            toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
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
