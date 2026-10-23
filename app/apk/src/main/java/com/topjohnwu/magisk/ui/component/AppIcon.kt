package com.topjohnwu.magisk.ui.component

import android.content.Context
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.graphics.drawable.Drawable
import android.util.LruCache
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import com.google.accompanist.drawablepainter.rememberDrawablePainter
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private class CachedIcon(val lastUpdateTime: Long, val drawable: Drawable)

// Keep recently shown icons so scrolling back through long app lists doesn't reload them
private val iconCache = LruCache<String, CachedIcon>(128)

/**
 * Returns the icon of [packageName], reusing the cached one unless the package has been
 * updated since it was loaded.
 */
private fun loadAppIcon(context: Context, packageName: String): Drawable {
    val pm = context.packageManager
    // Lists include apps that are only installed for other users
    val info = runCatching { pm.getPackageInfo(packageName, MATCH_UNINSTALLED_PACKAGES) }.getOrNull()
        ?: return pm.defaultActivityIcon
    iconCache[packageName]?.let { cached ->
        if (cached.lastUpdateTime == info.lastUpdateTime) return cached.drawable
    }
    val icon = info.applicationInfo?.loadIcon(pm) ?: pm.defaultActivityIcon
    iconCache.put(packageName, CachedIcon(info.lastUpdateTime, icon))
    return icon
}

@Composable
fun AppIcon(
    packageName: String,
    modifier: Modifier = Modifier,
    contentDescription: String? = null,
) {
    val context = LocalContext.current
    // Show the cached icon right away, then make sure it is still up to date
    var drawable by remember(packageName) { mutableStateOf(iconCache[packageName]?.drawable) }
    LaunchedEffect(packageName) {
        drawable = withContext(Dispatchers.IO) { loadAppIcon(context, packageName) }
    }
    val icon = drawable
    if (icon == null) {
        // Reserve the space while loading
        Box(modifier = modifier)
    } else {
        Image(
            painter = rememberDrawablePainter(icon),
            contentDescription = contentDescription,
            modifier = modifier,
        )
    }
}
