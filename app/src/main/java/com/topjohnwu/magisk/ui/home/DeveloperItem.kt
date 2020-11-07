package com.topjohnwu.magisk.ui.home

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.databinding.RvItem

sealed class DeveloperItem {

    abstract val items: List<IconLink>
    abstract val name: Int

    object Main : DeveloperItem() {
        override val items =
            listOf(
                IconLink.Twitter.Main,
                IconLink.Patreon,
                IconLink.PayPal.Main,
                IconLink.Github
            )
        override val name get() = R.string.topjohnwu
    }

    object App : DeveloperItem() {
        override val items =
            listOf<IconLink>(
                IconLink.Twitter.App,
                IconLink.PayPal.App
            )
        override val name get() = R.string.diareuse
    }
}

private interface Dev {
    val name: String
}

private interface MainDev: Dev {
    override val name get() = "topjohnwu"
}

private interface AppDev: Dev {
    override val name get() = "diareuse"
}

sealed class IconLink : RvItem() {

    abstract val icon: Int
    abstract val title: Int
    abstract val link: String

    override val layoutRes get() = R.layout.item_icon_link

    sealed class PayPal : IconLink(), Dev {
        override val icon get() = R.drawable.ic_paypal
        override val title get() = R.string.paypal
        override val link get() = "https://paypal.me/$name"

        object App : PayPal(), AppDev

        object Main : PayPal() {
            override val name: String get() = "magiskdonate"
        }
    }

    object Patreon : IconLink() {
        override val icon get() = R.drawable.ic_patreon
        override val title get() = R.string.patreon
        override val link get() = Const.Url.PATREON_URL
    }

    sealed class Twitter : IconLink(), Dev {
        override val icon get() = R.drawable.ic_twitter
        override val title get() = R.string.twitter
        override val link get() = "https://twitter.com/$name"

        object App : Twitter(), AppDev

        object Main : Twitter(), MainDev
    }

    object Github : IconLink() {
        override val icon get() = R.drawable.ic_github
        override val title get() = R.string.home_item_source
        override val link get() = Const.Url.SOURCE_CODE_URL
    }
}
