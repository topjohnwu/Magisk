package com.topjohnwu.magisk.core.base

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import androidx.collection.SparseArrayCompat
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ktx.set
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

    fun withPermission(permission: String, builder: PermissionRequestBuilder.() -> Unit) {
        val request = PermissionRequestBuilder().apply(builder).build()

        if (permission == Manifest.permission.WRITE_EXTERNAL_STORAGE &&
            Build.VERSION.SDK_INT >= 29) {
            // We do not need external rw on 29+
            request.onSuccess()
            return
        }

        if (ContextCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED) {
            request.onSuccess()
        } else {
            val requestCode = Random.nextInt(256, 512)
            resultCallbacks[requestCode] =  { result, _ ->
                if (result > 0)
                    request.onSuccess()
                else
                    request.onFailure()
            }
            ActivityCompat.requestPermissions(this, arrayOf(permission), requestCode)
        }
    }

    fun withExternalRW(builder: PermissionRequestBuilder.() -> Unit) {
        withPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE, builder = builder)
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
            it(this, if (success) 1 else -1, null)
        }

    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        resultCallbacks[requestCode]?.also {
            resultCallbacks.remove(requestCode)
            it(this, resultCode, data)
        }
    }

    fun startActivityForResult(intent: Intent, requestCode: Int, listener: RequestCallback) {
        resultCallbacks[requestCode] = listener
        startActivityForResult(intent, requestCode)
    }

    override fun recreate() {
        startActivity(Intent().setComponent(intent.component))
        finish()
    }

}
