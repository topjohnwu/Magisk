package com.topjohnwu.magisk.model.events

import com.skoumal.teanity.viewevents.ViewEvent


data class OpenLinkEvent(val url: String) : ViewEvent()

object ManagerInstallEvent : ViewEvent()
object MagiskInstallEvent : ViewEvent()

object ManagerChangelogEvent : ViewEvent()
object MagiskChangelogEvent : ViewEvent()

object UninstallEvent : ViewEvent()
