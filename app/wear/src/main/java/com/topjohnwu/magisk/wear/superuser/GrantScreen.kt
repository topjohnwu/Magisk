package com.topjohnwu.magisk.wear.superuser

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Shield
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.semantics.toggleableState
import androidx.compose.ui.state.ToggleableState
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.items
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.ButtonDefaults
import androidx.wear.compose.material3.CompactButton
import androidx.wear.compose.material3.FilledTonalButton
import androidx.wear.compose.material3.Icon
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.MaterialTheme
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.SwitchButton
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.ui.component.rememberBiometricAuth
import com.topjohnwu.magisk.ui.superuser.SuGrantAppState
import com.topjohnwu.magisk.ui.superuser.SuperuserGrantViewModel
import com.topjohnwu.magisk.wear.component.AppIcon
import com.topjohnwu.magisk.wear.component.LoadingScreen
import com.topjohnwu.magisk.core.R as CoreR

/**
 * Grants root to apps that have no policy yet. New entries reach the Superuser list through
 * the policy change event the view model sends, like on the phone.
 */
@Composable
fun GrantScreen(viewModel: SuperuserGrantViewModel) {
    val loading by viewModel.loading.collectAsStateWithLifecycle()
    val apps by viewModel.filteredApps.collectAsStateWithLifecycle()
    val showSystem by viewModel.showSystem.collectAsStateWithLifecycle()

    // The kind of root the next grant gives, switched with the mode chip. It starts at the
    // "restrict root capabilities" setting, which stays unchanged.
    var restrict by rememberSaveable { mutableStateOf(Config.suRestrict) }
    val requireAuth = rememberBiometricAuth()

    // The mode each app was granted with here, by uid, to show it in its row
    var grantedModes by rememberSaveable { mutableStateOf(HashMap<Int, Boolean>()) }

    // Kept across the loading screen, so the list doesn't scroll back when it ends
    val state = rememberTransformingLazyColumnState()

    if (loading) {
        LoadingScreen()
        return
    }

    // Apps granted before this screen opened are already in the Superuser list. Apps granted
    // here stay, so a mistaken grant can be undone right away.
    val candidates = apps.filter { !it.initiallyGranted }

    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader {
                    Text(stringResource(CoreR.string.superuser_grant))
                }
            }
            item {
                CompactButton(
                    onClick = { restrict = !restrict },
                    icon = { Icon(Icons.Filled.Shield, contentDescription = null) },
                    label = { Text(stringResource(modeLabel(restrict))) },
                    colors = ButtonDefaults.filledTonalButtonColors(),
                )
            }
            items(
                items = candidates,
                key = { it.info.packageName },
            ) { app ->
                GrantRow(
                    app = app,
                    grantedRestricted = grantedModes[app.info.uid],
                    onToggle = {
                        // The mode of this tap, whatever the chip shows once authenticated
                        val restricted = restrict
                        requireAuth {
                            if (!app.isGranted) {
                                grantedModes = HashMap(grantedModes).apply {
                                    put(app.info.uid, restricted)
                                }
                            }
                            // The view model flips the latest state, so quick repeated taps
                            // never get lost
                            viewModel.toggleGrant(app, restricted)
                        }
                    },
                )
            }
            item {
                SwitchButton(
                    checked = showSystem,
                    onCheckedChange = { viewModel.setShowSystem(it) },
                    label = { Text(stringResource(CoreR.string.show_system_app)) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

private fun modeLabel(restricted: Boolean) =
    if (restricted) CoreR.string.wear_root_restricted else CoreR.string.wear_root_full

@Composable
private fun GrantRow(
    app: SuGrantAppState,
    // The mode it was granted with on this screen, if known
    grantedRestricted: Boolean?,
    onToggle: () -> Unit,
) {
    val colors = MaterialTheme.colorScheme
    FilledTonalButton(
        onClick = onToggle,
        modifier = Modifier
            .fillMaxWidth()
            .semantics { toggleableState = ToggleableState(app.isGranted) },
        content = {
            AppIcon(packageName = app.info.packageName, size = 28.dp)
            Column(
                modifier = Modifier.weight(1f).padding(horizontal = 8.dp),
                verticalArrangement = Arrangement.Center,
            ) {
                Text(
                    text = app.info.label,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
                Text(
                    text = if (app.isGranted && grantedRestricted != null) {
                        stringResource(
                            CoreR.string.wear_grant_granted,
                            stringResource(modeLabel(grantedRestricted)),
                        )
                    } else {
                        app.info.packageName
                    },
                    style = MaterialTheme.typography.labelSmall,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                )
            }
            Box(
                modifier = Modifier
                    .size(32.dp)
                    .background(
                        color = if (app.isGranted) colors.primary else colors.primary.copy(alpha = 0.2f),
                        shape = CircleShape,
                    ),
                contentAlignment = Alignment.Center,
            ) {
                Icon(
                    imageVector = if (app.isGranted) Icons.Filled.Check else Icons.Filled.Add,
                    contentDescription = null,
                    tint = if (app.isGranted) colors.onPrimary else colors.primary,
                    modifier = Modifier.size(20.dp),
                )
            }
        },
    )
}
