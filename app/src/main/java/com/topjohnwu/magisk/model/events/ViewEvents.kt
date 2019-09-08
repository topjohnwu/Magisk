package com.topjohnwu.magisk.model.events

import android.app.Activity
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.model.entity.module.Repo
import io.reactivex.subjects.PublishSubject


data class OpenLinkEvent(val url: String) : ViewEvent()

class ManagerInstallEvent : ViewEvent()
class MagiskInstallEvent : ViewEvent()

class ManagerChangelogEvent : ViewEvent()
class MagiskChangelogEvent : ViewEvent()

class UninstallEvent : ViewEvent()
class EnvFixEvent : ViewEvent()

class UpdateSafetyNetEvent : ViewEvent()

class ViewActionEvent(val action: Activity.() -> Unit) : ViewEvent()

class OpenFilePickerEvent : ViewEvent()

class OpenChangelogEvent(val item: Repo) : ViewEvent()
class InstallModuleEvent(val item: Repo) : ViewEvent()

class PageChangedEvent : ViewEvent()

class PermissionEvent(
    val permissions: List<String>,
    val callback: PublishSubject<Boolean>
) : ViewEvent()

class BackPressEvent : ViewEvent()

class DieEvent : ViewEvent()