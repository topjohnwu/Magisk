package com.topjohnwu.magisk.core.base

import android.Manifest.permission.POST_NOTIFICATIONS
import android.Manifest.permission.REQUEST_INSTALL_PACKAGES
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Parcelable
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.result.ActivityResultCallback
import androidx.activity.result.contract.ActivityResultContracts.GetContent
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.ktx.reflectField
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.utils.RequestAuthentication
import com.topjohnwu.magisk.core.utils.RequestInstall

interface ContentResultCallback: ActivityResultCallback<Uri>, Parcelable {
    fun onActivityLaunch() {}
    // Make the result type explicitly non-null
    override fun onActivityResult(result: Uri)
}

interface UntrackedActivity

interface IActivityExtension {
    val extension: ActivityExtension
    fun withPermission(permission: String, callback: (Boolean) -> Unit) {
        extension.withPermission(permission, callback)
    }
    fun withAuthentication(callback: (Boolean) -> Unit) {
        extension.withAuthentication(callback)
    }
    fun getContent(type: String, callback: ContentResultCallback) {
        extension.getContent(type, callback)
    }
}

class ActivityExtension(private val activity: ComponentActivity) {

    private var permissionCallback: ((Boolean) -> Unit)? = null
    private val requestPermission = activity.registerForActivityResult(RequestPermission()) {
        permissionCallback?.invoke(it)
        permissionCallback = null
    }

    private var installCallback: ((Boolean) -> Unit)? = null
    private val requestInstall = activity.registerForActivityResult(RequestInstall()) {
        installCallback?.invoke(it)
        installCallback = null
    }

    private var authenticateCallback: ((Boolean) -> Unit)? = null
    private val requestAuthenticate = activity.registerForActivityResult(RequestAuthentication()) {
        authenticateCallback?.invoke(it)
        authenticateCallback = null
    }

    private var contentCallback: ContentResultCallback? = null
    private val getContent = activity.registerForActivityResult(GetContent()) {
        if (it != null) contentCallback?.onActivityResult(it)
        contentCallback = null
    }

    fun onCreate(savedInstanceState: Bundle?) {
        contentCallback = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.TIRAMISU) {
            savedInstanceState?.getParcelable(CONTENT_CALLBACK_KEY)
        } else {
            savedInstanceState
                ?.getParcelable(CONTENT_CALLBACK_KEY, ContentResultCallback::class.java)
        }
    }

    fun onSaveInstanceState(outState: Bundle) {
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

    fun withAuthentication(callback: (Boolean) -> Unit) {
        authenticateCallback = callback
        requestAuthenticate.launch(Unit)
    }

    fun getContent(type: String, callback: ContentResultCallback) {
        contentCallback = callback
        try {
            getContent.launch(type)
            callback.onActivityLaunch()
        } catch (e: ActivityNotFoundException) {
            activity.toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }

    companion object {
        private const val CONTENT_CALLBACK_KEY = "content_callback"
    }
}

val Activity.launchPackage: String? get() {
    return if (Build.VERSION.SDK_INT >= 34) {
        launchedFromPackage
    } else {
        Activity::class.java.reflectField("mReferrer").get(this) as String?
    }
}

fun Activity.relaunch() {
    startActivity(Intent(intent).setFlags(0))
    finish()
}
