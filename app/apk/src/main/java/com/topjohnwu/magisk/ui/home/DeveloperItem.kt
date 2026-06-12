package com.topjohnwu.magisk.ui.home

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.databinding.RvItem
import com.topjohnwu.magisk.core.R as CoreR

interface Dev {
    val name: String
}

private interface JohnImpl : Dev {
    override val name get() = "topjohnwu"
}

private interface VvbImpl : Dev {
    override val name get() = "vvb2060"
}

private interface YUImpl : Dev {
    override val name get() = "yujincheng08"
}

private interface RikkaImpl : Dev {
    override val name get() = "RikkaW"
}

private interface CanyieImpl : Dev {
    override val name get() = "canyie"
}

sealed class DeveloperItem : Dev {

    abstract val items: List<IconLink>
    val handle get() = "@${name}"

    object John : DeveloperItem(), JohnImpl {
        override val items =
            listOf(
                object : IconLink.Twitter(), JohnImpl {},
                IconLink.Github.Project
            )
    }

    object Vvb : DeveloperItem(), VvbImpl {
        override val items =
            listOf<IconLink>(
                object : IconLink.Twitter(), VvbImpl {},
                object : IconLink.Github.User(), VvbImpl {}
            )
    }

    object YU : DeveloperItem(), YUImpl {
        override val items =
            listOf<IconLink>(
                object : IconLink.Twitter() { override val name = "shanasaimoe" },
                object : IconLink.Github.User(), YUImpl {},
                object : IconLink.Sponsor(), YUImpl {}
            )
    }

    object Rikka : DeveloperItem(), RikkaImpl {
        override val items =
            listOf<IconLink>(
                object : IconLink.Twitter() { override val name = "rikkawww" },
                object : IconLink.Github.User(), RikkaImpl {}
            )
    }

    object Canyie : DeveloperItem(), CanyieImpl {
        override val items =
            listOf<IconLink>(
                object : IconLink.Twitter() { override val name = "canyie2977" },
                object : IconLink.Github.User(), CanyieImpl {}
            )
    }
}

sealed class IconLink : RvItem() {

    abstract val icon: Int
    abstract val title: Int
    abstract val link: String

    override val layoutRes get() = R.layout.item_icon_link

    abstract class PayPal : IconLink(), Dev {
        override val icon get() = CoreR.drawable.ic_paypal
        override val title get() = CoreR.string.paypal
        override val link get() = "https://paypal.me/$name"

        object Project : PayPal() {
            override val name: String get() = "magiskdonate"
        }
    }

    object Patreon : IconLink() {
        override val icon get() = CoreR.drawable.ic_patreon
        override val title get() = CoreR.string.patreon
        override val link get() = Const.Url.PATREON_URL
    }

    abstract class Twitter : IconLink(), Dev {
        override val icon get() = CoreR.drawable.ic_twitter
        override val title get() = CoreR.string.twitter
        override val link get() = "https://twitter.com/$name"
    }

    abstract class Github : IconLink() {
        override val icon get() = CoreR.drawable.ic_github
        override val title get() = CoreR.string.github

        abstract class User : Github(), Dev {
            override val link get() = "https://github.com/$name"
        }

        object Project : Github() {
            override val link get() = Const.Url.SOURCE_CODE_URL
        }
    }

    abstract class Sponsor : IconLink(), Dev {
        override val icon get() = CoreR.drawable.ic_favorite
        override val title get() = CoreR.string.github
        override val link get() = "https://github.com/sponsors/$name"
    }
}
