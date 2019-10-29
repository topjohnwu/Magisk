package com.topjohnwu.magisk.ui.surequest

import android.content.pm.ActivityInfo
import android.os.Build
import android.os.Bundle
import android.view.Window
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.base.BaseActivity
import com.topjohnwu.magisk.databinding.ActivityRequestBinding
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.model.receiver.GeneralReceiver
import com.topjohnwu.magisk.utils.SuLogger
import org.koin.androidx.viewmodel.ext.android.viewModel

open class SuRequestActivity : BaseActivity<SuRequestViewModel, ActivityRequestBinding>() {

    override val layoutRes: Int = R.layout.activity_request
    override val themeRes: Int = R.style.MagiskTheme_SU
    override val viewModel: SuRequestViewModel by viewModel()

    override fun onBackPressed() {
        viewModel.handler?.handleAction(MagiskPolicy.DENY, -1)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        lockOrientation()
        super.onCreate(savedInstanceState)

        val intent = intent

        when (intent?.action) {
            GeneralReceiver.REQUEST -> {
                if (!viewModel.handleRequest(intent))
                    finish()
                return
            }
            GeneralReceiver.LOG -> SuLogger.handleLogs(this, intent)
            GeneralReceiver.NOTIFY -> SuLogger.handleNotify(this, intent)
        }

        finish()
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is DieEvent -> finish()
        }
    }

    private fun lockOrientation() {
        requestedOrientation = if (Build.VERSION.SDK_INT < 18)
            resources.configuration.orientation
        else
            ActivityInfo.SCREEN_ORIENTATION_LOCKED
    }
}
