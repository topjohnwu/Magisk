package com.topjohnwu.magisk.ui.module

import android.annotation.SuppressLint
import android.content.pm.ActivityInfo
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.activity.OnBackPressedCallback
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
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.ui.theme.MagiskTheme
import com.topjohnwu.magisk.core.R as CoreR

class ActionFragment : Fragment(), ViewModelHolder {

    override val viewModel by lazy {
        ViewModelProvider(this, VMFactory)[ActionViewModel::class.java]
    }

    private var defaultOrientation = -1

    private val backCallback = object : OnBackPressedCallback(true) {
        override fun handleOnBackPressed() {
            if ((viewModel as ActionViewModel).state.value != ActionViewModel.State.RUNNING) {
                isEnabled = false
                activity?.onBackPressedDispatcher?.onBackPressed()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        startObserveLiveData()
        (viewModel as ActionViewModel).args = ActionFragmentArgs.fromBundle(requireArguments())
        activity?.onBackPressedDispatcher?.addCallback(this, backCallback)
    }

    override fun onStart() {
        super.onStart()
        val vm = viewModel as ActionViewModel
        (activity as? NavigationActivity<*>)?.setTitle(vm.args.name)

        vm.state.observe(this) {
            if (it == ActionViewModel.State.SUCCESS) {
                context?.toast(
                    getString(CoreR.string.done_action, vm.args.name),
                    Toast.LENGTH_SHORT
                )
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        defaultOrientation = activity?.requestedOrientation ?: -1
        activity?.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED

        if (savedInstanceState == null) {
            (viewModel as ActionViewModel).startRunAction()
        }

        return ComposeView(requireContext()).apply {
            setViewCompositionStrategy(ViewCompositionStrategy.DisposeOnViewTreeLifecycleDestroyed)
            setContent {
                MagiskTheme {
                    ActionScreen(
                        viewModel = viewModel as ActionViewModel,
                        onClose = { activity?.onBackPressedDispatcher?.onBackPressed() }
                    )
                }
            }
        }
    }

    @SuppressLint("WrongConstant")
    override fun onDestroyView() {
        if (defaultOrientation != -1) {
            activity?.requestedOrientation = defaultOrientation
        }
        super.onDestroyView()
    }

    override fun onEventDispatched(event: ViewEvent) {
        when (event) {
            is ContextExecutor -> event(requireContext())
            is ActivityExecutor -> (activity as? UIActivity<*>)?.let { event(it) }
        }
    }
}
