package com.topjohnwu.magisk.legacy.flash

import android.content.Context
import android.content.Intent
import android.content.pm.ActivityInfo
import android.net.Uri
import android.os.Bundle
import androidx.core.net.toUri
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.magisk.databinding.ActivityFlashBinding
import com.topjohnwu.magisk.extensions.snackbar
import com.topjohnwu.magisk.model.events.PermissionEvent
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.ui.base.BaseUIActivity
import com.topjohnwu.magisk.ui.base.CompatNavigationDelegate
import org.koin.androidx.viewmodel.ext.android.viewModel
import java.io.File

open class FlashActivity : BaseUIActivity<FlashViewModel, ActivityFlashBinding>() {

    override val layoutRes: Int = R.layout.activity_flash
    override val viewModel: FlashViewModel by viewModel()

    override val navigation: CompatNavigationDelegate<BaseUIActivity<FlashViewModel, ActivityFlashBinding>>? =
        null

    override fun onCreate(savedInstanceState: Bundle?) {
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_NOSENSOR
        super.onCreate(savedInstanceState)
        val id = intent.getIntExtra(Const.Key.DISMISS_ID, -1)
        if (id != -1)
            Notifications.mgr.cancel(id)
        viewModel.startFlashing(intent)
    }

    override fun onBackPressed() {
        if (viewModel.loading) return
        super.onBackPressed()
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is SnackbarEvent -> snackbar(snackbarView, event.message(this), event.length, event.f)
            is PermissionEvent -> withPermissions(*event.permissions.toTypedArray()) {
                onSuccess { event.callback.onNext(true) }
                onFailure {
                    event.callback.onNext(false)
                    event.callback.onError(SecurityException("User refused permissions"))
                }
            }
        }
    }

    companion object {

        private fun intent(context: Context) = context.intent<FlashActivity>()
            .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        private fun intent(context: Context, file: File) = intent(context).setData(file.toUri())

        private fun flashType(isSecondSlot: Boolean) =
            if (isSecondSlot) Const.Value.FLASH_INACTIVE_SLOT else Const.Value.FLASH_MAGISK

        /* Flashing is understood as installing / flashing magisk itself */

        fun flashIntent(context: Context, file: File, isSecondSlot: Boolean, id : Int = -1)
            = intent(context, file)
            .putExtra(Const.Key.FLASH_ACTION, flashType(isSecondSlot))
            .putExtra(Const.Key.DISMISS_ID, id)

        fun flash(context: Context, file: File, isSecondSlot: Boolean, id: Int) =
            context.startActivity(flashIntent(context, file, isSecondSlot, id))

        /* Patching is understood as injecting img files with magisk */

        fun patchIntent(context: Context, file: File, uri: Uri, id : Int = -1)
            = intent(context, file)
            .putExtra(Const.Key.FLASH_DATA, uri)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_FILE)
            .putExtra(Const.Key.DISMISS_ID, id)

        fun patch(context: Context, file: File, uri: Uri, id: Int) =
            context.startActivity(patchIntent(context, file, uri, id))

        /* Uninstalling is understood as removing magisk entirely */

        fun uninstallIntent(context: Context, file: File, id : Int = -1)
            = intent(context, file)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL)
            .putExtra(Const.Key.DISMISS_ID, id)

        fun uninstall(context: Context, file: File, id: Int) =
            context.startActivity(uninstallIntent(context, file, id))

        /* Installing is understood as flashing modules / zips */

        fun installIntent(context: Context, file: File, id : Int = -1)
            = intent(context, file)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP)
            .putExtra(Const.Key.DISMISS_ID, id)

        fun install(context: Context, file: File, id: Int) =
            context.startActivity(installIntent(context, file, id))

    }

}
