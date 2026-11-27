package com.topjohnwu.magisk.wear.component

import android.content.Context
import android.content.pm.PackageManager.MATCH_UNINSTALLED_PACKAGES
import android.util.LruCache
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.size
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.Dp
import androidx.core.graphics.drawable.toBitmap
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private class CachedIcon(val lastUpdateTime: Long, val bitmap: ImageBitmap)

// Keep recently shown icons so scrolling back through app lists doesn't reload them
private val iconCache = LruCache<String, CachedIcon>(64)

private fun cacheKey(packageName: String, sizePx: Int) = "$packageName:$sizePx"

/**
 * Returns the icon of [packageName] drawn at [sizePx], reusing the cached one unless the
 * package has been updated since it was loaded. Icons are drawn once at their display size,
 * since the watch shows them small.
 */
private fun loadAppIcon(context: Context, packageName: String, sizePx: Int): ImageBitmap {
    val pm = context.packageManager
    // Lists include apps that are only installed for other users
    val info = runCatching { pm.getPackageInfo(packageName, MATCH_UNINSTALLED_PACKAGES) }.getOrNull()
        ?: return pm.defaultActivityIcon.toBitmap(sizePx, sizePx).asImageBitmap()
    val key = cacheKey(packageName, sizePx)
    iconCache[key]?.let { cached ->
        if (cached.lastUpdateTime == info.lastUpdateTime) return cached.bitmap
    }
    val drawable = info.applicationInfo?.loadIcon(pm) ?: pm.defaultActivityIcon
    val bitmap = drawable.toBitmap(sizePx, sizePx).asImageBitmap()
    iconCache.put(key, CachedIcon(info.lastUpdateTime, bitmap))
    return bitmap
}

@Composable
fun AppIcon(
    packageName: String,
    size: Dp,
    modifier: Modifier = Modifier,
    contentDescription: String? = null,
) {
    val context = LocalContext.current
    val sizePx = with(LocalDensity.current) { size.roundToPx() }.coerceAtLeast(1)
    // Show the cached icon right away, then make sure it is still up to date
    var bitmap by remember(packageName, sizePx) {
        mutableStateOf(iconCache[cacheKey(packageName, sizePx)]?.bitmap)
    }
    LaunchedEffect(packageName, sizePx) {
        bitmap = withContext(Dispatchers.IO) { loadAppIcon(context, packageName, sizePx) }
    }
    val icon = bitmap
    if (icon == null) {
        // Reserve the space while loading
        Box(modifier = modifier.size(size))
    } else {
        Image(
            bitmap = icon,
            contentDescription = contentDescription,
            modifier = modifier.size(size),
        )
    }
}
