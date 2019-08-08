package com.topjohnwu.magisk.ui.surequest

import android.content.pm.ActivityInfo
import android.os.Build
import android.os.Bundle
import android.text.TextUtils
import android.view.Window
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityRequestBinding
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.receiver.GeneralReceiver
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.utils.SuLogger
import org.koin.androidx.viewmodel.ext.android.viewModel

open class SuRequestActivity : MagiskActivity<SuRequestViewModel, ActivityRequestBinding>() {

    override val layoutRes: Int = R.layout.activity_request
    override val viewModel: SuRequestViewModel by viewModel()

    override fun onBackPressed() {
        viewModel.handler?.handleAction(MagiskPolicy.DENY, -1)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        lockOrientation()
        super.onCreate(savedInstanceState)

        val intent = intent
        val action = intent.action

        if (TextUtils.equals(action, GeneralReceiver.REQUEST)) {
            if (!viewModel.handleRequest(intent))
                finish()
            return
        }

        if (TextUtils.equals(action, GeneralReceiver.LOG))
            SuLogger.handleLogs(intent)
        else if (TextUtils.equals(action, GeneralReceiver.NOTIFY))
            SuLogger.handleNotify(intent)

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
