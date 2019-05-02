package com.topjohnwu.magisk.ui.surequest

import android.hardware.fingerprint.FingerprintManager
import android.os.CountDownTimer
import android.text.SpannableStringBuilder
import android.widget.Toast
import androidx.core.text.bold
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivitySuRequestBinding
import com.topjohnwu.magisk.model.entity.Policy
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.SuDialogEvent
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.utils.FingerprintHelper
import com.topjohnwu.magisk.utils.feature.WIP
import com.topjohnwu.magisk.view.MagiskDialog
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.koin.core.parameter.parametersOf
import timber.log.Timber
import java.util.concurrent.TimeUnit.MILLISECONDS
import java.util.concurrent.TimeUnit.SECONDS

@WIP
open class _SuRequestActivity : MagiskActivity<_SuRequestViewModel, ActivitySuRequestBinding>() {

    override val layoutRes: Int = R.layout.activity_su_request
    override val viewModel: _SuRequestViewModel by viewModel {
        parametersOf(intent, intent.action)
    }

    //private val timeoutPrefs: SharedPreferences by inject(SUTimeout)
    private val canUseFingerprint get() = FingerprintHelper.useFingerprint()

    private val countdown by lazy {
        val seconds = Config.get<Int>(Config.Key.SU_REQUEST_TIMEOUT).toLong()
        val millis = SECONDS.toMillis(seconds)
        object : CountDownTimer(millis, 1000) {
            override fun onFinish() {
                viewModel.deny()
            }

            override fun onTick(millisUntilFinished: Long) {
                dialog.applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                    Timber.e("Tick, tock")
                    title = "%s (%d)".format(
                        getString(R.string.deny),
                        MILLISECONDS.toSeconds(millisUntilFinished)
                    )
                }
            }
        }
    }

    private var fingerprintHelper: SuFingerprint? = null

    private lateinit var dialog: MagiskDialog

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is SuDialogEvent -> showDialog(event.policy)
            is DieEvent -> finish()
        }
    }

    override fun onBackPressed() {
        if (::dialog.isInitialized && dialog.isShowing) {
            return
        }
        super.onBackPressed()
    }

    override fun onDestroy() {
        if (this::dialog.isInitialized && dialog.isShowing) {
            dialog.dismiss()
        }
        fingerprintHelper?.cancel()
        countdown.cancel()
        super.onDestroy()
    }

    private fun showDialog(policy: Policy) {
        val titleText = SpannableStringBuilder("Allow ")
            .bold { append(policy.appName) }
            .append(" to access superuser rights?")

        val messageText = StringBuilder()
            .appendln(policy.packageName)
            .append(getString(R.string.su_warning))

        dialog = MagiskDialog(this)
            .applyIcon(policy.info.loadIcon(packageManager))
            .applyTitle(titleText)
            .applyMessage(messageText)
            //.applyView()) {} //todo add a spinner
            .cancellable(false)
            .applyButton(MagiskDialog.ButtonType.POSITIVE) {
                titleRes = R.string.grant
                onClick { viewModel.grant() }
                if (canUseFingerprint) {
                    icon = R.drawable.ic_fingerprint
                }
            }
            .applyButton(MagiskDialog.ButtonType.NEUTRAL) {
                title = "%s %s".format(getString(R.string.grant), getString(R.string.once))
                onClick { viewModel.grant(-1) }
            }
            .applyButton(MagiskDialog.ButtonType.NEGATIVE) {
                titleRes = R.string.deny
                onClick { viewModel.deny() }
            }
            .onDismiss { finish() }
            .onShow {
                startTimer().also { Timber.e("Starting timer") }
                if (canUseFingerprint) {
                    startFingerprintQuery()
                }
            }
            .reveal()
    }

    private fun startTimer() {
        countdown.start()
    }

    private fun startFingerprintQuery() {
        val result = runCatching {
            fingerprintHelper = SuFingerprint().apply { authenticate() }
        }

        if (result.isFailure) {
            dialog.applyButton(MagiskDialog.ButtonType.POSITIVE) {
                icon = 0
            }
        }
    }

    private inner class SuFingerprint @Throws(Exception::class)
    internal constructor() : FingerprintHelper() {

        override fun onAuthenticationError(errorCode: Int, errString: CharSequence) {
            Toast.makeText(this@_SuRequestActivity, errString, Toast.LENGTH_LONG).show()
        }

        override fun onAuthenticationHelp(helpCode: Int, helpString: CharSequence) {
            Toast.makeText(this@_SuRequestActivity, helpString, Toast.LENGTH_LONG).show()
        }

        override fun onAuthenticationSucceeded(result: FingerprintManager.AuthenticationResult) {
            viewModel.grant()
        }

        override fun onAuthenticationFailed() {
            Toast.makeText(this@_SuRequestActivity, R.string.auth_fail, Toast.LENGTH_LONG).show()
        }
    }

    companion object {

        const val REQUEST = "request"
        const val LOG = "log"
        const val NOTIFY = "notify"
    }
}
