package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.res.Resources
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.Window
import android.view.WindowManager
import androidx.activity.compose.setContent
import androidx.databinding.ViewDataBinding
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.core.base.UntrackedActivity
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.SuCallbackHandler.REQUEST
import com.topjohnwu.magisk.ui.theme.MagiskTheme
import com.topjohnwu.magisk.ui.theme.Theme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

open class SuRequestActivity : UIActivity<ViewDataBinding>(), UntrackedActivity {

    override val layoutRes: Int = 0
    override val viewModel: SuRequestViewModel by viewModel()

    override val snackbarView: View
        get() = window.decorView.findViewById(android.R.id.content)

    override fun onCreate(savedInstanceState: Bundle?) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.addFlags(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            window.setHideOverlayWindows(true)
        }
        setTheme(Theme.selected.themeRes)
        super.onCreate(savedInstanceState)

        if (intent.action == Intent.ACTION_VIEW) {
            val action = intent.getStringExtra("action")
            if (action == REQUEST) {
                viewModel.handleRequest(intent)
            } else {
                lifecycleScope.launch {
                    withContext(Dispatchers.IO) {
                        SuCallbackHandler.run(this@SuRequestActivity, action, intent.extras)
                    }
                    finish()
                }
            }
        } else {
            finish()
        }

        if (viewModel.useTapjackProtection) {
            window.decorView.rootView.accessibilityDelegate =
                SuRequestViewModel.EmptyAccessibilityDelegate
        }

        setContent {
            MagiskTheme {
                SuRequestScreen(viewModel = viewModel)
            }
        }
    }

    override fun getTheme(): Resources.Theme {
        val theme = super.getTheme()
        theme.applyStyle(R.style.Foundation_Floating, true)
        return theme
    }

    @Deprecated("Use OnBackPressedDispatcher")
    override fun onBackPressed() {
        viewModel.denyPressed()
    }

    override fun finish() {
        super.finishAndRemoveTask()
    }
}
