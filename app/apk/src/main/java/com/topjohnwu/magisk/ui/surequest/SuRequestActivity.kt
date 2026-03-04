package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.res.Resources
import android.os.Build
import android.os.Bundle
import android.view.Window
import android.view.WindowManager
import androidx.activity.compose.setContent
import androidx.appcompat.app.AppCompatDelegate
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.core.base.UntrackedActivity
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.SuCallbackHandler.REQUEST
import com.topjohnwu.magisk.databinding.ActivityRequestBinding
import com.topjohnwu.magisk.events.ShowUIEvent
import com.topjohnwu.magisk.ui.theme.Theme
import com.topjohnwu.magisk.ui.compose.surequest.SuRequestScreen
import com.topjohnwu.magisk.ui.compose.surequest.suRequestColorScheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import com.topjohnwu.magisk.arch.ViewEvent

open class SuRequestActivity : UIActivity<ActivityRequestBinding>(), UntrackedActivity {

    override val layoutRes: Int = R.layout.activity_request
    override val viewModel: SuRequestViewModel by viewModel()
    private val useCompose = BuildConfig.COMPOSE_UI
    private val composeVisible = mutableStateOf(false)

    override fun onCreate(savedInstanceState: Bundle?) {
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.addFlags(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            window.setHideOverlayWindows(true)
        }
        setTheme(Theme.selected.themeRes)

        if (useCompose) {
            setContent {
                val darkMode = when (Config.darkTheme) {
                    AppCompatDelegate.MODE_NIGHT_YES,
                    Config.Value.DARK_THEME_AMOLED -> true
                    AppCompatDelegate.MODE_NIGHT_NO -> false
                    else -> androidx.compose.foundation.isSystemInDarkTheme()
                }
                val dynamic = Theme.shouldUseDynamicColor
                val scheme = suRequestColorScheme(useDynamicColor = dynamic, darkTheme = darkMode)

                MaterialTheme(colorScheme = scheme) {
                    val showUi by remember { composeVisible }
                    SuRequestScreen(
                        viewModel = viewModel,
                        showContent = showUi,
                        onTimeoutSelected = { viewModel.selectedItemPosition = it },
                        onSpinnerTouched = { viewModel.spinnerTouched() },
                        onGrant = { viewModel.grantPressed() },
                        onDeny = { viewModel.denyPressed() }
                    )
                }
            }
        }

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
    }

    override fun getTheme(): Resources.Theme {
        val theme = super.getTheme()
        theme.applyStyle(R.style.Foundation_Floating, true)
        return theme
    }

    override fun onBackPressed() {
        viewModel.denyPressed()
    }

    override fun finish() {
        super.finishAndRemoveTask()
    }

    override fun onEventDispatched(event: ViewEvent) {
        if (useCompose && event is ShowUIEvent) {
            setAccessibilityDelegate(event.delegate)
            composeVisible.value = true
            return
        }
        super.onEventDispatched(event)
    }
}
