package com.topjohnwu.magisk.ui.component

import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.utils.RequestAuthentication

class BiometricAuth internal constructor(
    private val launch: (force: Boolean, action: () -> Unit) -> Unit
) : ((() -> Unit) -> Unit) {
    override fun invoke(action: () -> Unit) {
        launch(false, action)
    }

    operator fun invoke(force: Boolean, action: () -> Unit) {
        launch(force, action)
    }
}

/**
 * Returns a lifecycle-safe authenticator that prompts for biometric or device credentials
 * (PIN, pattern, password) before executing sensitive actions.
 *
 * Scoped directly to the Composable lifecycle using [rememberLauncherForActivityResult],
 * ensuring zero Activity references are leaked to ViewModels or retained singletons.
 */
@Composable
fun rememberBiometricAuth(): BiometricAuth {
    var pendingAction by remember { mutableStateOf<(() -> Unit)?>(null) }
    val launcher = rememberLauncherForActivityResult(RequestAuthentication()) { success ->
        if (success) {
            pendingAction?.invoke()
        }
        pendingAction = null
    }
    return remember(launcher) {
        BiometricAuth { force, action ->
            if ((force || Config.suAuth) && Info.isDeviceSecure) {
                pendingAction = action
                launcher.launch(Unit)
            } else {
                action()
            }
        }
    }
}
