package com.topjohnwu.magisk.base

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.collection.SparseArrayCompat
import androidx.core.net.toUri
import androidx.databinding.DataBindingUtil
import androidx.databinding.ViewDataBinding
import com.karumi.dexter.Dexter
import com.karumi.dexter.MultiplePermissionsReport
import com.karumi.dexter.PermissionToken
import com.karumi.dexter.listener.PermissionRequest
import com.karumi.dexter.listener.multi.MultiplePermissionsListener
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.base.viewmodel.MagiskViewModel
import com.topjohnwu.magisk.extensions.set
import com.topjohnwu.magisk.model.events.EventHandler
import com.topjohnwu.magisk.model.permissions.PermissionRequestBuilder
import com.topjohnwu.magisk.utils.LocaleManager
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.utils.currentLocale

typealias RequestCallback = MagiskActivity<*, *>.(Int, Intent?) -> Unit

abstract class MagiskActivity<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
        AppCompatActivity(), EventHandler {

    protected lateinit var binding: Binding
    protected abstract val layoutRes: Int
    protected abstract val viewModel: ViewModel
    protected open val snackbarView get() = binding.root
    protected open val navHostId: Int = 0
    protected open val defaultPosition: Int = 0

    private val resultCallbacks = SparseArrayCompat<RequestCallback>()

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
        super.attachBaseContext(LocaleManager.getLocaleContext(base))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        viewModel.viewEvents.observe(this, viewEventObserver)

        binding = DataBindingUtil.setContentView<Binding>(this, layoutRes).apply {
            setVariable(BR.viewModel, viewModel)
            lifecycleOwner = this@MagiskActivity
        }
    }

    fun openUrl(url: String) = Utils.openLink(this, url.toUri())

    fun withPermissions(vararg permissions: String, builder: PermissionRequestBuilder.() -> Unit) {
        val request = PermissionRequestBuilder().apply(builder).build()
        Dexter.withActivity(this)
            .withPermissions(*permissions)
            .withListener(object : MultiplePermissionsListener {
                override fun onPermissionsChecked(report: MultiplePermissionsReport) {
                    if (report.areAllPermissionsGranted()) {
                        request.onSuccess()
                    } else {
                        request.onFailure()
                    }
                }

                override fun onPermissionRationaleShouldBeShown(
                    permissions: MutableList<PermissionRequest>,
                    token: PermissionToken
                ) = token.continuePermissionRequest()
            }).check()
    }

    fun withExternalRW(builder: PermissionRequestBuilder.() -> Unit) {
        withPermissions(Manifest.permission.WRITE_EXTERNAL_STORAGE, builder = builder)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        resultCallbacks[requestCode]?.apply {
            resultCallbacks.remove(requestCode)
            invoke(this@MagiskActivity, resultCode, data)
        }
    }

    fun startActivityForResult(intent: Intent, requestCode: Int, listener: RequestCallback) {
        resultCallbacks[requestCode] = listener
        startActivityForResult(intent, requestCode)
    }

}
