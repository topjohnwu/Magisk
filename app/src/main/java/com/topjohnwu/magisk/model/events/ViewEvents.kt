package com.topjohnwu.magisk.model.events

import android.app.Activity
import com.skoumal.teanity.viewevents.ViewEvent


data class OpenLinkEvent(val url: String) : ViewEvent()

class ManagerInstallEvent : ViewEvent()
class MagiskInstallEvent : ViewEvent()

class ManagerChangelogEvent : ViewEvent()
class MagiskChangelogEvent : ViewEvent()

class UninstallEvent : ViewEvent()
class EnvFixEvent : ViewEvent()

class UpdateSafetyNetEvent : ViewEvent()

class ViewActionEvent(val action: Activity.() -> Unit) : ViewEvent()