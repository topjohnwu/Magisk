package com.topjohnwu.magisk.wear.files

import android.content.ActivityNotFoundException
import android.content.pm.ApplicationInfo
import android.net.Uri
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.wear.compose.foundation.lazy.TransformingLazyColumn
import androidx.wear.compose.foundation.lazy.items
import androidx.wear.compose.foundation.lazy.rememberTransformingLazyColumnState
import androidx.wear.compose.material3.Button
import androidx.wear.compose.material3.ListHeader
import androidx.wear.compose.material3.ScreenScaffold
import androidx.wear.compose.material3.Text
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.wear.nav.LocalWearNavigator
import com.topjohnwu.magisk.wear.nav.WearNavigator
import com.topjohnwu.magisk.wear.nav.WearRoute
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.launch
import timber.log.Timber
import com.topjohnwu.magisk.core.R as CoreR

enum class FileKind(val mimeTypes: Array<String>, val extensions: Set<String>) {
    ZIP(arrayOf("application/zip"), setOf("zip")),
    // Boot images, and Odin firmware packages that contain them
    IMAGE(arrayOf("application/octet-stream", "application/x-tar"), setOf("img", "tar", "tar.md5")),
}

/**
 * Returns a function that lets the user pick a file of [kind] and passes it to [onPicked].
 *
 * The system picker (ACTION_OPEN_DOCUMENT) is used when a system app handles it. Most watches
 * have none, only a stub that says so (see [documentPickerPackage]), so otherwise the
 * [WearRoute.Pick] screen lists the files in the download folder through the root shell. Its
 * result comes back through [WearNavigator.setPickResult].
 */
@Composable
fun rememberFilePicker(kind: FileKind, onPicked: (Uri) -> Unit): () -> Unit {
    val context = LocalContext.current
    val navigator = LocalWearNavigator.current
    val currentOnPicked by rememberUpdatedState(onPicked)

    val launcher = rememberLauncherForActivityResult(OpenDocumentIn()) { uri ->
        uri?.let { currentOnPicked(it) }
    }

    val rootResult = navigator.pickResult(kind)
    LaunchedEffect(rootResult) {
        if (rootResult != null) {
            navigator.consumePickResult(kind)
            currentOnPicked(rootResult)
        }
    }

    return remember(kind, context, navigator, launcher) {
        {
            val intent = ActivityResultContracts.OpenDocument()
                .createIntent(context, kind.mimeTypes)
            val handlers = context.packageManager.queryIntentActivities(intent, 0).map {
                val app = it.activityInfo.applicationInfo
                PickerHandler(app.packageName, (app.flags and ApplicationInfo.FLAG_SYSTEM) != 0)
            }
            val picker = documentPickerPackage(handlers)
            val launched = picker != null && try {
                launcher.launch(DocumentRequest(picker, kind.mimeTypes))
                true
            } catch (_: ActivityNotFoundException) {
                // The handler can be disabled between the check and the launch
                false
            }
            if (!launched) navigator.navigate(WearRoute.Pick(kind))
        }
    }
}

/** Files of [kind] in the download folder, listed with the root shell. */
@Composable
fun PickScreen(kind: FileKind, navigator: WearNavigator) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()
    // null while loading
    var files by remember { mutableStateOf<List<String>?>(null) }
    var copying by remember { mutableStateOf(false) }

    LaunchedEffect(kind) {
        files = RootFiles.list(ext = kind.extensions)
    }

    val state = rememberTransformingLazyColumnState()
    ScreenScaffold(state) { contentPadding ->
        TransformingLazyColumn(state = state, contentPadding = contentPadding) {
            item {
                ListHeader { Text(stringResource(CoreR.string.select_from_storage)) }
            }
            val current = files
            when {
                current == null -> item {
                    Text(stringResource(CoreR.string.loading), textAlign = TextAlign.Center)
                }
                current.isEmpty() -> item {
                    Text(stringResource(CoreR.string.wear_pick_empty), textAlign = TextAlign.Center)
                }
                else -> items(current) { name ->
                    Button(
                        onClick = {
                            if (copying) return@Button
                            copying = true
                            scope.launch {
                                try {
                                    val uri = RootFiles.copyToCache(context, name)
                                    // Only deliver it if the user hasn't swiped away meanwhile
                                    val pick = WearRoute.Pick(kind)
                                    if (navigator.current() == pick) {
                                        navigator.setPickResult(kind, uri)
                                        navigator.pop()
                                    }
                                } catch (e: CancellationException) {
                                    throw e
                                } catch (e: Exception) {
                                    Timber.e(e)
                                    context.toast(CoreR.string.failure, Toast.LENGTH_SHORT)
                                } finally {
                                    copying = false
                                }
                            }
                        },
                        enabled = !copying,
                        modifier = Modifier.fillMaxWidth(),
                    ) {
                        Text(name)
                    }
                }
            }
        }
    }
}
