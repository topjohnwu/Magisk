package com.topjohnwu.magisk.ui.base

import android.Manifest
import android.content.Intent
import androidx.collection.SparseArrayCompat
import androidx.databinding.ViewDataBinding
import com.karumi.dexter.Dexter
import com.karumi.dexter.MultiplePermissionsReport
import com.karumi.dexter.PermissionToken
import com.karumi.dexter.listener.PermissionRequest
import com.karumi.dexter.listener.multi.MultiplePermissionsListener
import com.skoumal.teanity.view.TeanityActivity
import com.topjohnwu.magisk.Const

abstract class MagiskLeanbackActivity<ViewModel : MagiskViewModel, Binding : ViewDataBinding> :
    TeanityActivity<ViewModel, Binding>(), IBaseLeanback {

    private val resultListeners = SparseArrayCompat<ActivityResultListener>()

    @Deprecated("Permissions will be checked in a different streamlined way")
    fun runWithExternalRW(callback: () -> Unit) = runWithExternalRW(Runnable { callback() })

    @Deprecated("Permissions will be checked in a different streamlined way")
    override fun runWithExternalRW(callback: Runnable) {
        runWithPermissions(Manifest.permission.WRITE_EXTERNAL_STORAGE, callback = callback)
    }

    @Deprecated("Permissions will be checked in a different streamlined way")
    override fun runWithPermissions(vararg permissions: String, callback: Runnable) {
        Dexter.withActivity(this)
            .withPermissions(*permissions)
            .withListener(object : MultiplePermissionsListener {
                override fun onPermissionsChecked(report: MultiplePermissionsReport?) {
                    if (report?.areAllPermissionsGranted() == true) {
                        Const.EXTERNAL_PATH.mkdirs()
                        callback.run()
                    }
                }

                override fun onPermissionRationaleShouldBeShown(
                    permissions: MutableList<PermissionRequest>?,
                    token: PermissionToken?
                ) = Unit
            })
            .check()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        resultListeners.get(requestCode)?.apply {
            resultListeners.remove(requestCode)
            onActivityResult(resultCode, data)
        }
    }

    override fun startActivityForResult(
        intent: Intent,
        requestCode: Int,
        listener: ActivityResultListener
    ) {
        resultListeners.put(requestCode, listener)
        startActivityForResult(intent, requestCode)
    }
}