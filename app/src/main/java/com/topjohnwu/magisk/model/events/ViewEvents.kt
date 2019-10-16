package com.topjohnwu.magisk.model.events

import android.app.Activity
import androidx.appcompat.app.AppCompatActivity
import com.karumi.dexter.Dexter
import com.karumi.dexter.MultiplePermissionsReport
import com.karumi.dexter.PermissionToken
import com.karumi.dexter.listener.PermissionRequest
import com.karumi.dexter.listener.multi.MultiplePermissionsListener
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.model.entity.module.Repo
import com.topjohnwu.magisk.model.permissions.PermissionRequestBuilder
import io.reactivex.subjects.PublishSubject

/**
 * Class for passing events from ViewModels to Activities/Fragments
 * Variable [handled] used so each event is handled only once
 * (see https://medium.com/google-developers/livedata-with-snackbar-navigation-and-other-events-the-singleliveevent-case-ac2622673150)
 * Use [ViewEventObserver] for observing these events
 */
abstract class ViewEvent {

    var handled = false
}

data class OpenLinkEvent(val url: String) : ViewEvent()

class ManagerInstallEvent : ViewEvent()
class MagiskInstallEvent : ViewEvent()

class ManagerChangelogEvent : ViewEvent()
class MagiskChangelogEvent : ViewEvent()

class UninstallEvent : ViewEvent()
class EnvFixEvent : ViewEvent()

class UpdateSafetyNetEvent : ViewEvent()

class ViewActionEvent(val action: Activity.() -> Unit) : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: AppCompatActivity) = activity.run(action)
}

class OpenFilePickerEvent : ViewEvent()

class OpenChangelogEvent(val item: Repo) : ViewEvent()
class InstallModuleEvent(val item: Repo) : ViewEvent()

class PageChangedEvent : ViewEvent()

class PermissionEvent(
    val permissions: List<String>,
    val callback: PublishSubject<Boolean>
) : ViewEvent(), ActivityExecutor {

    private val permissionRequest = PermissionRequestBuilder().apply {
        onSuccess {
            callback.onNext(true)
        }
        onFailure {
            callback.onNext(false)
            callback.onError(SecurityException("User refused permissions"))
        }
    }.build()

    override fun invoke(activity: AppCompatActivity) = Dexter.withActivity(activity)
        .withPermissions(permissions)
        .withListener(object : MultiplePermissionsListener {
            override fun onPermissionRationaleShouldBeShown(
                permissions: MutableList<PermissionRequest>,
                token: PermissionToken
            ) = token.continuePermissionRequest()

            override fun onPermissionsChecked(
                report: MultiplePermissionsReport
            ) = if (report.areAllPermissionsGranted()) {
                permissionRequest.onSuccess()
            } else {
                permissionRequest.onFailure()
            }
        }).check()
}

class BackPressEvent : ViewEvent(), ActivityExecutor {
    override fun invoke(activity: AppCompatActivity) {
        activity.onBackPressed()
    }
}

class DieEvent : ViewEvent()
