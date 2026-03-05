package com.topjohnwu.magisk.ui.surequest

import android.content.Context
import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.res.Resources
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.view.WindowManager
import android.view.accessibility.AccessibilityEvent
import android.view.accessibility.AccessibilityNodeInfo
import android.view.accessibility.AccessibilityNodeProvider
import androidx.activity.compose.setContent
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.app.AppCompatDelegate
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.Modifier
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.base.ActivityExtension
import com.topjohnwu.magisk.core.base.UntrackedActivity
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.SuCallbackHandler.REQUEST
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ui.theme.Theme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

open class SuRequestActivity : AppCompatActivity(), UntrackedActivity {

    private val extension = ActivityExtension(this)
    private val viewModel: SuRequestViewModel by lazy {
        ViewModelProvider(this, VMFactory)[SuRequestViewModel::class.java]
    }

    init {
        val nightMode = if (Config.darkTheme == Config.Value.DARK_THEME_AMOLED) {
            AppCompatDelegate.MODE_NIGHT_YES
        } else {
            Config.darkTheme
        }
        AppCompatDelegate.setDefaultNightMode(nightMode)
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        extension.onCreate(savedInstanceState)
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.addFlags(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            window.setHideOverlayWindows(true)
        }
        setTheme(Theme.selected.themeRes)
        super.onCreate(savedInstanceState)

        viewModel.finishActivity = { finish() }
        viewModel.authenticate = { onSuccess ->
            extension.withAuthentication { if (it) onSuccess() }
        }

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
            window.decorView.rootView.accessibilityDelegate = EmptyAccessibilityDelegate
        }

        setContent {
            val (darkMode, useDynamicColor) = resolveSuRequestTheme(isSystemInDarkTheme())
            MaterialTheme(
                colorScheme = suRequestColorScheme(
                    useDynamicColor = useDynamicColor,
                    darkTheme = darkMode
                )
            ) {
                Box(modifier = Modifier.fillMaxSize()) {
                    SuRequestScreen(viewModel = viewModel)
                }
            }
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        extension.onSaveInstanceState(outState)
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

    private fun resolveSuRequestTheme(systemDark: Boolean): Pair<Boolean, Boolean> {
        val monetSupported = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
        return when (Config.colorMode) {
            1 -> false to false
            2 -> true to false
            3 -> systemDark to monetSupported
            4 -> false to monetSupported
            5 -> true to monetSupported
            else -> {
                val darkMode = when (Config.darkTheme) {
                    AppCompatDelegate.MODE_NIGHT_YES,
                    Config.Value.DARK_THEME_AMOLED -> true

                    AppCompatDelegate.MODE_NIGHT_NO -> false
                    else -> systemDark
                }
                darkMode to Theme.shouldUseDynamicColor
            }
        }
    }

    private object EmptyAccessibilityDelegate : View.AccessibilityDelegate() {
        override fun sendAccessibilityEvent(host: View, eventType: Int) {}
        override fun performAccessibilityAction(host: View, action: Int, args: Bundle?) = true
        override fun sendAccessibilityEventUnchecked(host: View, event: AccessibilityEvent) {}
        override fun dispatchPopulateAccessibilityEvent(host: View, event: AccessibilityEvent) =
            true

        override fun onPopulateAccessibilityEvent(host: View, event: AccessibilityEvent) {}
        override fun onInitializeAccessibilityEvent(host: View, event: AccessibilityEvent) {}
        override fun onInitializeAccessibilityNodeInfo(host: View, info: AccessibilityNodeInfo) {}
        override fun addExtraDataToAccessibilityNodeInfo(
            host: View,
            info: AccessibilityNodeInfo,
            extraDataKey: String,
            arguments: Bundle?
        ) {
        }

        override fun onRequestSendAccessibilityEvent(
            host: ViewGroup,
            child: View,
            event: AccessibilityEvent
        ): Boolean = false

        override fun getAccessibilityNodeProvider(host: View): AccessibilityNodeProvider? = null
    }
}
