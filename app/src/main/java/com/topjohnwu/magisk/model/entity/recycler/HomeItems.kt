package com.topjohnwu.magisk.model.entity.recycler

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.databinding.ComparableRvItem

sealed class HomeItem : ComparableRvItem<HomeItem>() {

    abstract val icon: Int
    abstract val title: Int
    abstract val link: String

    override val layoutRes = R.layout.item_developer_link

    override fun contentSameAs(other: HomeItem) = itemSameAs(other)
    override fun itemSameAs(other: HomeItem) = this == other

    override fun equals(other: Any?): Boolean {
        if (other !is HomeItem) return false
        return icon == other.icon && title == other.title && link == other.link
    }

    override fun hashCode() =
        icon.hashCode() + title.hashCode() + link.hashCode() + layoutRes.hashCode()

    // region Children
    sealed class PayPal : HomeItem() {
        override val icon = R.drawable.ic_paypal
        override val title = R.string.paypal
        override val link = "https://paypal.me/%s"

        // region Children
        object App : PayPal() {
            override val link = super.link.format("diareuse")
        }

        object Mainline : PayPal() {
            override val link = super.link.format("topjohnwu")
        }
        // endregion
    }

    object Patreon : HomeItem() {
        override val icon = R.drawable.ic_patreon
        override val title = R.string.patreon
        override val link = Const.Url.PATREON_URL
    }

    sealed class Twitter : HomeItem() {
        override val icon = R.drawable.ic_twitter
        override val title = R.string.twitter
        override val link = "https://twitter.com/%s"

        // region Children
        object App : Twitter() {
            override val link = super.link.format("diareuse")
        }

        object Mainline : Twitter() {
            override val link = super.link.format("topjohnwu")
        }
        // endregion
    }

    object Github : HomeItem() {
        override val icon = R.drawable.ic_github
        override val title = R.string.home_item_source
        override val link = Const.Url.SOURCE_CODE_URL
    }

    object Xda : HomeItem() {
        override val icon = R.drawable.ic_xda
        override val title = R.string.xda
        override val link = Const.Url.XDA_THREAD
    }
    // endregion
}

sealed class DeveloperItem : ComparableRvItem<DeveloperItem>() {

    abstract val items: List<HomeItem>
    abstract val name: Int

    override val layoutRes = R.layout.item_developer

    override fun contentSameAs(other: DeveloperItem) = itemSameAs(other)
    override fun itemSameAs(other: DeveloperItem) = this == other

    override fun equals(other: Any?): Boolean {
        if (other !is DeveloperItem) return false
        return name == other.name && items == other.items
    }

    override fun hashCode() = name.hashCode() + items.hashCode() + layoutRes.hashCode()

    //region Children
    object Mainline : DeveloperItem() {
        override val items =
            listOf(HomeItem.PayPal.Mainline, HomeItem.Patreon, HomeItem.Twitter.Mainline)
        override val name = R.string.topjohnwu
    }

    object App : DeveloperItem() {
        override val items =
            listOf(HomeItem.PayPal.App, HomeItem.Twitter.App)
        override val name = R.string.diareuse
    }

    object Project : DeveloperItem() {
        override val items =
            listOf(HomeItem.Github, HomeItem.Xda)
        override val name = R.string.home_links_project
    }
    //endregion

}
