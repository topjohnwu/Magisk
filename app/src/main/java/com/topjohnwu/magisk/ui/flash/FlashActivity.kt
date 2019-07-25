package com.topjohnwu.magisk.ui.flash

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import androidx.core.app.NotificationManagerCompat
import androidx.core.net.toUri
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityFlashBinding
import com.topjohnwu.magisk.ui.base.MagiskActivity
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.koin.core.parameter.parametersOf
import java.io.File

open class FlashActivity : MagiskActivity<FlashViewModel, ActivityFlashBinding>() {

    override val layoutRes: Int = R.layout.activity_flash
    override val viewModel: FlashViewModel by viewModel {
        val uri = intent.data ?: let { finish(); Uri.EMPTY }
        val additionalUri = intent.getParcelableExtra(Const.Key.FLASH_DATA) ?: uri
        val action = intent.getStringExtra(Const.Key.FLASH_ACTION) ?: let { finish();"" }
        parametersOf(action, uri, additionalUri)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val id = intent.getIntExtra(Const.Key.DISMISS_ID, -1)
        if (id != -1)
            NotificationManagerCompat.from(this).cancel(id)
    }

    override fun onBackPressed() {
        if (viewModel.loading) return
        super.onBackPressed()
    }

    companion object {

        private fun intent(context: Context) = Intent(context, ClassMap[FlashActivity::class.java])
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
