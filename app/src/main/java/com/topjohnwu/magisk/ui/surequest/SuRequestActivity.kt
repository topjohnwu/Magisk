package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.pm.ActivityInfo
import android.os.Build
import android.os.Bundle
import android.view.Window
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.base.BaseActivity
import com.topjohnwu.magisk.databinding.ActivityRequestBinding
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.utils.SuHandler
import com.topjohnwu.magisk.utils.SuHandler.REQUEST
import org.koin.androidx.viewmodel.ext.android.viewModel

open class SuRequestActivity : BaseActivity<SuRequestViewModel, ActivityRequestBinding>() {

    override val layoutRes: Int = R.layout.activity_request
    override val themeRes: Int = R.style.MagiskTheme_SU
    override val viewModel: SuRequestViewModel by viewModel()

    override fun onBackPressed() {
        viewModel.denyPressed()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        lockOrientation()
        super.onCreate(savedInstanceState)

        fun showRequest() {
            if (!viewModel.handleRequest(intent))
                finish()
        }

        fun runHandler(action: String?) {
            SuHandler(this, action, intent.extras)
            finish()
        }

        if (intent.action == Intent.ACTION_VIEW) {
            val action = intent.getStringExtra("action")
            if (action == REQUEST) {
                showRequest()
            } else {
                runHandler(action)
            }
        } else if (intent.action == REQUEST) {
            showRequest()
        } else {
            runHandler(intent.action)
        }
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is ViewActionEvent -> event.action(this)
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
