package com.topjohnwu.magisk.ui.navigation

import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalLifecycleOwner
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.BaseViewModel
import com.topjohnwu.magisk.arch.ContextExecutor
import com.topjohnwu.magisk.arch.UIActivity
import com.topjohnwu.magisk.arch.ViewEvent

@Composable
fun ObserveViewEvents(viewModel: BaseViewModel) {
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val activity = context as? UIActivity

    DisposableEffect(viewModel, lifecycleOwner) {
        val observer = { event: ViewEvent ->
            when (event) {
                is ContextExecutor -> event(context)
                is ActivityExecutor -> activity?.let { event(it) }
            }
        }
        viewModel.viewEvents.observe(lifecycleOwner, observer)
        onDispose {
            viewModel.viewEvents.removeObserver(observer)
        }
    }
}

@Composable
fun CollectNavEvents(viewModel: BaseViewModel, navigator: Navigator) {
    LaunchedEffect(viewModel) {
        viewModel.navEvents.collect { route ->
            navigator.push(route)
        }
    }
}
