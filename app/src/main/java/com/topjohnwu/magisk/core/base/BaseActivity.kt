package com.topjohnwu.magisk.core.base

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.Configuration
import androidx.appcompat.app.AppCompatActivity
import androidx.collection.SparseArrayCompat
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.extensions.set
import com.topjohnwu.magisk.model.permissions.PermissionRequestBuilder
import kotlin.random.Random

typealias RequestCallback = BaseActivity.(Int, Intent?) -> Unit

abstract class BaseActivity : AppCompatActivity() {

    private val resultCallbacks by lazy { SparseArrayCompat<RequestCallback>() }

    override fun applyOverrideConfiguration(config: Configuration?) {
        // Force applying our preferred local
        config?.setLocale(currentLocale)
        super.applyOverrideConfiguration(config)
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap(false))
    }

    fun withPermissions(vararg permissions: String, builder: PermissionRequestBuilder.() -> Unit) {
        val request = PermissionRequestBuilder().apply(builder).build()
        val ungranted = permissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }

        if (ungranted.isEmpty()) {
            request.onSuccess()
        } else {
            val requestCode = Random.nextInt(256, 512)
            resultCallbacks[requestCode] =  { result, _ ->
                if (result > 0)
                    request.onSuccess()
                else
                    request.onFailure()
            }
            ActivityCompat.requestPermissions(this, ungranted.toTypedArray(), requestCode)
        }
    }

    fun withExternalRW(builder: PermissionRequestBuilder.() -> Unit) {
        withPermissions(Manifest.permission.WRITE_EXTERNAL_STORAGE, builder = builder)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        var success = true
        for (res in grantResults) {
            if (res != PackageManager.PERMISSION_GRANTED) {
                success = false
                break
            }
        }
        resultCallbacks[requestCode]?.also {
            resultCallbacks.remove(requestCode)
            it(this@BaseActivity, if (success) 1 else -1, null)
        }

    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        resultCallbacks[requestCode]?.also {
            resultCallbacks.remove(requestCode)
            it(this@BaseActivity, resultCode, data)
        }
    }

    fun startActivityForResult(intent: Intent, requestCode: Int, listener: RequestCallback) {
        resultCallbacks[requestCode] = listener
        startActivityForResult(intent, requestCode)
    }

    override fun recreate() {
        startActivity(intent)
        finish()
    }

}
