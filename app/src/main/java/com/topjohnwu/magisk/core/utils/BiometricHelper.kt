package com.topjohnwu.magisk.core.utils

import android.app.Activity
import android.app.KeyguardManager
import android.content.Context
import android.content.Intent
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import com.topjohnwu.magisk.core.Config

class BiometricHelper(context: Context) {

    private val mgr = context.getSystemService(KeyguardManager::class.java)

    val isSupported get() = mgr.isDeviceSecure

    val isEnabled get() = isSupported && Config.suBiometric

    fun authenticate(
            activity: FragmentActivity,
            onError: () -> Unit = {},
            onSuccess: () -> Unit) {
        val tag = BiometricFragment::class.java.name
        val intent = mgr.createConfirmDeviceCredentialIntent(null, null)
        val fragmentManager = activity.supportFragmentManager
        var fragment = fragmentManager.findFragmentByTag(tag) as BiometricFragment?
        if (fragment == null) {
            fragment = BiometricFragment()
            fragmentManager.beginTransaction()
                    .add(0, fragment, tag)
                    .commitNow()
        }
        fragment.start(intent, onError, onSuccess)
    }

    class BiometricFragment : Fragment() {
        private val code = 1
        private var onError: () -> Unit = {}
        private var onSuccess: () -> Unit = {}
        override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
            if (requestCode == code) {
                    if (resultCode == Activity.RESULT_OK) {
                        onSuccess()
                    } else {
                        onError()
                    }
                    onError = {}
                    onSuccess = {}
            }
        }

        fun start(intent: Intent, onError: () -> Unit, onSuccess: () -> Unit) {
            this.onError = onError
            this.onSuccess = onSuccess
            intent.addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK or Intent.FLAG_ACTIVITY_NEW_DOCUMENT)
            startActivityForResult(intent, code)
        }
    }
}
