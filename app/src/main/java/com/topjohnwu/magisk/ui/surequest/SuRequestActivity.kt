package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.res.Resources
import android.os.Bundle
import android.view.Window
import android.view.WindowManager
import androidx.navigation.NavController
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.SuCallbackHandler.REQUEST
import com.topjohnwu.magisk.databinding.ActivityRequestBinding
import com.topjohnwu.magisk.di.viewModel

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
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.addFlags(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN)
        super.onCreate(savedInstanceState)

        fun showRequest() {
            viewModel.handleRequest(intent)
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

    private fun lockOrientation() {
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
    }
}
