package com.topjohnwu.magisk.model.events

import android.content.Context
import android.content.res.Resources
import android.util.TypedValue
import androidx.annotation.AttrRes
import androidx.browser.customtabs.CustomTabsIntent
import androidx.core.net.toUri
import com.topjohnwu.magisk.R

data class OpenInappLinkEvent(
    private val link: String
) : ViewEvent(), ContextExecutor {

    // todo find app that can open the link and as a fallback open custom tabs! it shouldn't be the default
    override fun invoke(context: Context) = CustomTabsIntent.Builder()
        .setShowTitle(true)
        .setToolbarColor(context.themedColor(R.attr.colorSurface))
        .enableUrlBarHiding()
        .build()
        .launchUrl(context, link.toUri())

    private fun Context.themedColor(@AttrRes attribute: Int) = theme
        .resolveAttribute(attribute).data

    private fun Resources.Theme.resolveAttribute(
        @AttrRes attribute: Int,
        resolveRefs: Boolean = true
    ) = TypedValue().also { resolveAttribute(attribute, it, resolveRefs) }

}