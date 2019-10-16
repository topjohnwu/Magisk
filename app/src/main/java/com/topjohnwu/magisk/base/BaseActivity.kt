package com.topjohnwu.magisk.base

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.collection.SparseArrayCompat
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.base.viewmodel.BaseViewModel
import com.topjohnwu.magisk.extensions.set
import com.topjohnwu.magisk.model.events.EventHandler
import com.topjohnwu.magisk.model.permissions.PermissionRequestBuilder
import com.topjohnwu.magisk.utils.currentLocale
import com.topjohnwu.magisk.wrap
import kotlin.random.Random

typealias RequestCallback = BaseActivity<*, *>.(Int, Intent?) -> Unit

abstract class BaseActivity<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
        AppCompatActivity(), EventHandler {

    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected abstract val viewModel: ViewModel
    protected open val themeRes: Int = R.style.MagiskTheme
    protected open val snackbarView get() = binding.root

    private val resultCallbacks by lazy { SparseArrayCompat<RequestCallback>() }

    init {
        val theme = if (Config.darkTheme) {
            AppCompatDelegate.MODE_NIGHT_YES
        } else {
            AppCompatDelegate.MODE_NIGHT_NO
        }
        AppCompatDelegate.setDefaultNightMode(theme)
    }

    override fun applyOverrideConfiguration(config: Configuration?) {
        // Force applying our preferred local
        config?.setLocale(currentLocale)
        super.applyOverrideConfiguration(config)
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap(false))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        setTheme(themeRes)
        super.onCreate(savedInstanceState)

        viewModel.viewEvents.observe(this, viewEventObserver)

        binding = DataBindingUtil.setContentView<Binding>(this, layoutRes).apply {
            setVariable(BR.viewModel, viewModel)
            lifecycleOwner = this@BaseActivity
        }
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
        resultCallbacks[requestCode]?.apply {
            resultCallbacks.remove(requestCode)
            invoke(this@BaseActivity, if (success) 1 else -1, null)
        }

    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        resultCallbacks[requestCode]?.apply {
            resultCallbacks.remove(requestCode)
            invoke(this@BaseActivity, resultCode, data)
        }
    }

    fun startActivityForResult(intent: Intent, requestCode: Int, listener: RequestCallback) {
        resultCallbacks[requestCode] = listener
        startActivityForResult(intent, requestCode)
    }

}
