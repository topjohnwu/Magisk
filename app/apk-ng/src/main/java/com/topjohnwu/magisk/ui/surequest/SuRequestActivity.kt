package com.topjohnwu.magisk.ui.surequest

import android.content.Context
import android.content.Intent
import android.content.pm.ActivityInfo
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.view.WindowManager
import android.view.accessibility.AccessibilityEvent
import android.view.accessibility.AccessibilityNodeInfo
import android.view.accessibility.AccessibilityNodeProvider
import androidx.activity.ComponentActivity
import androidx.activity.addCallback
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.ui.Modifier
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.base.ActivityExtension
import com.topjohnwu.magisk.core.base.UntrackedActivity
import com.topjohnwu.magisk.core.su.SuCallbackHandler
import com.topjohnwu.magisk.core.su.SuCallbackHandler.REQUEST
import com.topjohnwu.magisk.core.wrap
import com.topjohnwu.magisk.ui.MagiskTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class SuRequestActivity : ComponentActivity(), UntrackedActivity {

    private val extension = ActivityExtension(this)
    private val viewModel: SuRequestViewModel by lazy {
        ViewModelProvider(this, VMFactory)[SuRequestViewModel::class.java]
    }

    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base.wrap())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        extension.onCreate(savedInstanceState)
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
        window.requestFeature(Window.FEATURE_NO_TITLE)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.addFlags(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            window.setHideOverlayWindows(true)
        }
        setTheme(R.style.Floating)
        super.onCreate(savedInstanceState)

        onBackPressedDispatcher.addCallback(this) { viewModel.denyPressed() }
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
            MagiskTheme {
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

    override fun finish() {
        super.finishAndRemoveTask()
    }

    private object EmptyAccessibilityDelegate : View.AccessibilityDelegate() {
        override fun sendAccessibilityEvent(host: View, eventType: Int) {}
        override fun performAccessibilityAction(host: View, action: Int, args: Bundle?) = true
        override fun sendAccessibilityEventUnchecked(host: View, event: AccessibilityEvent) {}
        override fun dispatchPopulateAccessibilityEvent(host: View, event: AccessibilityEvent) = true
        override fun onPopulateAccessibilityEvent(host: View, event: AccessibilityEvent) {}
        override fun onInitializeAccessibilityEvent(host: View, event: AccessibilityEvent) {}
        override fun onInitializeAccessibilityNodeInfo(host: View, info: AccessibilityNodeInfo) {}
        override fun addExtraDataToAccessibilityNodeInfo(host: View, info: AccessibilityNodeInfo, extraDataKey: String, arguments: Bundle?) {}
        override fun onRequestSendAccessibilityEvent(host: ViewGroup, child: View, event: AccessibilityEvent): Boolean = false
        override fun getAccessibilityNodeProvider(host: View): AccessibilityNodeProvider? = null
    }
}
