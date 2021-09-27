package com.topjohnwu.magisk.core.base

import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.os.Build
import android.os.Bundle
import android.widget.Toast
import androidx.annotation.CallSuper
import androidx.appcompat.app.AppCompatActivity
import androidx.collection.SparseArrayCompat
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ktx.reflectField
import com.topjohnwu.magisk.ktx.set
import com.topjohnwu.magisk.utils.Utils
import kotlin.random.Random

typealias ActivityResultCallback = BaseActivity.(Int, Intent?) -> Unit

abstract class BaseActivity : AppCompatActivity() {

    private val resultCallbacks by lazy { SparseArrayCompat<ActivityResultCallback>() }
    private val newRequestCode: Int get() {
        var requestCode: Int
        do {
            requestCode = Random.nextInt(0, 1 shl 15)
        } while (resultCallbacks.containsKey(requestCode))
        return requestCode
    }

    override fun applyOverrideConfiguration(config: Configuration?) {
        // Force applying our preferred local
        config?.setLocale(currentLocale)
        super.applyOverrideConfiguration(config)
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap(true))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        // Overwrite private members to avoid nasty "false" stack traces being logged
        val delegate = delegate
        val clz = delegate.javaClass
        clz.reflectField("mActivityHandlesUiModeChecked").set(delegate, true)
        clz.reflectField("mActivityHandlesUiMode").set(delegate, false)
        super.onCreate(savedInstanceState)
    }

    fun withPermission(permission: String, builder: PermissionRequestBuilder.() -> Unit) {
        val request = PermissionRequestBuilder().apply(builder).build()

        if (permission == WRITE_EXTERNAL_STORAGE && Build.VERSION.SDK_INT >= 30) {
            // We do not need external rw on 30+
            request.onSuccess()
            return
        }

        if (ContextCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED) {
            request.onSuccess()
        } else {
            val requestCode = newRequestCode
            resultCallbacks[requestCode] = { result, _ ->
                if (result > 0)
                    request.onSuccess()
                else
                    request.onFailure()
            }
            ActivityCompat.requestPermissions(this, arrayOf(permission), requestCode)
        }
    }

    fun withExternalRW(builder: PermissionRequestBuilder.() -> Unit) {
        withPermission(WRITE_EXTERNAL_STORAGE, builder = builder)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        var success = true
        for (res in grantResults) {
            if (res != PackageManager.PERMISSION_GRANTED) {
                success = false
                break
            }
        }
        resultCallbacks[requestCode]?.also {
            resultCallbacks.remove(requestCode)
            it(this, if (success) 1 else -1, null)
        }

    }

    @CallSuper
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        resultCallbacks[requestCode]?.also { callback ->
            resultCallbacks.remove(requestCode)
            callback(this, resultCode, data)
        }
    }

    fun startActivityForResult(intent: Intent, callback: ActivityResultCallback) {
        val requestCode = newRequestCode
        resultCallbacks[requestCode] = callback
        try {
            startActivityForResult(intent, requestCode)
        } catch (e: ActivityNotFoundException) {
            Utils.toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }

    override fun recreate() {
        startActivity(Intent().setComponent(intent.component))
        finish()
    }

}
