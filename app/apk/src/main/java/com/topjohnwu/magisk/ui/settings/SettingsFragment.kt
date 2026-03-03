package com.topjohnwu.magisk.ui.settings

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.ViewCompositionStrategy
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.arch.ViewModelHolder
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ui.theme.MagiskTheme
import com.topjohnwu.magisk.core.R as CoreR

class SettingsFragment : Fragment(), ViewModelHolder {

    override val viewModel by lazy {
        ViewModelProvider(this, VMFactory)[SettingsViewModel::class.java]
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        startObserveLiveData()
    }

    override fun onStart() {
        super.onStart()
        (activity as? NavigationActivity<*>)?.setTitle(CoreR.string.settings)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return ComposeView(requireContext()).apply {
            setViewCompositionStrategy(ViewCompositionStrategy.DisposeOnViewTreeLifecycleDestroyed)
            setContent {
                MagiskTheme {
                    SettingsScreen(viewModel = viewModel as SettingsViewModel)
                }
            }
        }
    }

    override fun onEventDispatched(event: ViewEvent) {
        when (event) {
            is ContextExecutor -> event(requireContext())
            is ActivityExecutor -> (activity as? UIActivity<*>)?.let { event(it) }
        }
    }
}
