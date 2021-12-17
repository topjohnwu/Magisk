package com.topjohnwu.magisk.core.base

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.net.Uri
import android.os.Build
import android.os.Bundle
import androidx.activity.result.contract.ActivityResultContracts.GetContent
import androidx.activity.result.contract.ActivityResultContracts.RequestPermission
import androidx.appcompat.app.AppCompatActivity
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ktx.reflectField

abstract class BaseActivity : AppCompatActivity() {

    private var permissionCallback: ((Boolean) -> Unit)? = null
    private val requestPermission = registerForActivityResult(RequestPermission()) {
        permissionCallback?.invoke(it)
        permissionCallback = null
    }

    private var contentCallback: ((Uri) -> Unit)? = null
    private val getContent = registerForActivityResult(GetContent()) {
        if (it != null) contentCallback?.invoke(it)
        contentCallback = null
    }

    override fun applyOverrideConfiguration(config: Configuration?) {
        // Force applying our preferred local
        config?.setLocale(currentLocale)
        super.applyOverrideConfiguration(config)
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
        super.onCreate(savedInstanceState)
    }

    fun withPermission(permission: String, callback: (Boolean) -> Unit) {
        if (permission == WRITE_EXTERNAL_STORAGE && Build.VERSION.SDK_INT >= 30) {
            // We do not need external rw on 30+
            callback(true)
            return
        }
        permissionCallback = callback
        requestPermission.launch(permission)
    }

    fun getContent(type: String, callback: (Uri) -> Unit) {
        contentCallback = callback
        getContent.launch(type)
    }

    override fun recreate() {
        startActivity(Intent().setComponent(intent.component))
        finish()
    }

    fun relaunch() {
        startActivity(Intent(intent).setFlags(0))
        finish()
    }
}
