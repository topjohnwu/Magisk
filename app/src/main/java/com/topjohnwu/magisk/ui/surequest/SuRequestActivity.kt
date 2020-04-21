package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.res.Resources
import android.os.Build
import android.os.Bundle
import android.view.Window
import android.view.WindowManager
import androidx.navigation.NavController
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.SuCallbackHandler.REQUEST
import com.topjohnwu.magisk.databinding.ActivityRequestBinding
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.model.events.ViewActionEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.ui.base.BaseUIActivity
import org.koin.androidx.viewmodel.ext.android.viewModel

open class SuRequestActivity : BaseUIActivity<SuRequestViewModel, ActivityRequestBinding>() {

    override val layoutRes: Int = R.layout.activity_request
    override val viewModel: SuRequestViewModel by viewModel()

    override val navigation: NavController? = null

    override fun onBackPressed() {
        viewModel.denyPressed()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        lockOrientation()
        window.setFlags(WindowManager.LayoutParams.FLAG_SECURE,
                WindowManager.LayoutParams.FLAG_SECURE)
        super.onCreate(savedInstanceState)

        fun showRequest() {
            if (!viewModel.handleRequest(intent))
                finish()
        }

        fun runHandler(action: String?) {
            SuCallbackHandler(this, action, intent.extras)
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

    override fun getTheme(): Resources.Theme {
        val theme = super.getTheme()
        theme.applyStyle(R.style.Foundation_Floating, true)
        return theme
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
