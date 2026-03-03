package com.topjohnwu.magisk.ui.flash

import android.annotation.SuppressLint
import android.content.Context
import android.content.pm.ActivityInfo
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.OnBackPressedCallback
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.ViewCompositionStrategy
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.NavDeepLinkBuilder
import com.topjohnwu.magisk.MainDirections
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.arch.ViewModelHolder
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.cmp
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.theme.MagiskTheme
import com.topjohnwu.magisk.core.R as CoreR

class FlashFragment : Fragment(), ViewModelHolder {

    override val viewModel by lazy {
        ViewModelProvider(this, VMFactory)[FlashViewModel::class.java]
    }

    private var defaultOrientation = -1

    private val backCallback = object : OnBackPressedCallback(true) {
        override fun handleOnBackPressed() {
            if ((viewModel as FlashViewModel).flashing.value != true) {
                isEnabled = false
                activity?.onBackPressedDispatcher?.onBackPressed()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        startObserveLiveData()
        (viewModel as FlashViewModel).args = FlashFragmentArgs.fromBundle(requireArguments())
        activity?.onBackPressedDispatcher?.addCallback(this, backCallback)
    }

    override fun onStart() {
        super.onStart()
        (activity as? NavigationActivity<*>)?.setTitle(CoreR.string.flash_screen_title)

        (viewModel as FlashViewModel).state.observe(this) {
            (activity as? androidx.appcompat.app.AppCompatActivity)?.supportActionBar?.setSubtitle(
                when (it) {
                    FlashViewModel.State.FLASHING -> CoreR.string.flashing
                    FlashViewModel.State.SUCCESS -> CoreR.string.done
                    FlashViewModel.State.FAILED -> CoreR.string.failure
                }
            )
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
            (viewModel as FlashViewModel).startFlashing()
        }

        return ComposeView(requireContext()).apply {
            setViewCompositionStrategy(ViewCompositionStrategy.DisposeOnViewTreeLifecycleDestroyed)
            setContent {
                MagiskTheme {
                    FlashScreen(viewModel = viewModel as FlashViewModel)
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

    companion object {

        private fun createIntent(context: Context, args: FlashFragmentArgs) =
            NavDeepLinkBuilder(context)
                .setGraph(R.navigation.main)
                .setComponentName(MainActivity::class.java.cmp(context.packageName))
                .setDestination(R.id.flashFragment)
                .setArguments(args.toBundle())
                .createPendingIntent()

        private fun flashType(isSecondSlot: Boolean) =
            if (isSecondSlot) Const.Value.FLASH_INACTIVE_SLOT else Const.Value.FLASH_MAGISK

        fun flash(isSecondSlot: Boolean) = MainDirections.actionFlashFragment(
            action = flashType(isSecondSlot)
        )

        fun patch(uri: Uri) = MainDirections.actionFlashFragment(
            action = Const.Value.PATCH_FILE,
            additionalData = uri
        )

        fun uninstall() = MainDirections.actionFlashFragment(
            action = Const.Value.UNINSTALL
        )

        fun installIntent(context: Context, file: Uri) = FlashFragmentArgs(
            action = Const.Value.FLASH_ZIP,
            additionalData = file,
        ).let { createIntent(context, it) }

        fun install(file: Uri) = MainDirections.actionFlashFragment(
            action = Const.Value.FLASH_ZIP,
            additionalData = file,
        )
    }
}
