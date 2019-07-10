package com.topjohnwu.magisk.ui.flash

import android.content.Context
import android.content.Intent
import android.net.Uri
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
        val additionalUri = intent.getParcelableExtra<Uri>(Const.Key.FLASH_DATA) ?: uri
        val action = intent.getStringExtra(Const.Key.FLASH_ACTION) ?: let { finish();"" }
        parametersOf(action, uri, additionalUri)
    }

    override fun onBackPressed() {
        if (viewModel.loading) return
        super.onBackPressed()
    }

    companion object {

        private fun intent(context: Context) = Intent(context, ClassMap[FlashActivity::class.java])
        private fun intent(context: Context, file: File) = intent(context).setData(file.toUri())

        /* Flashing is understood as installing / flashing magisk itself */

        fun flashIntent(context: Context, file: File) = intent(context, file)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK)

        fun flash(context: Context, file: File) =
            context.startActivity(flashIntent(context, file))

        /* Patching is understood as injecting img files with magisk */

        fun patchIntent(context: Context, file: File, uri: Uri) = intent(context, file)
            .putExtra(Const.Key.FLASH_DATA, uri)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_FILE)

        fun patch(context: Context, file: File, uri: Uri) =
            context.startActivity(patchIntent(context, file, uri))

        /* Uninstalling is understood as removing magisk entirely */

        fun uninstallIntent(context: Context, file: File) = intent(context, file)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.UNINSTALL)

        fun uninstall(context: Context, file: File) =
            context.startActivity(uninstallIntent(context, file))

        /* Installing is understood as flashing modules / zips */

        fun installIntent(context: Context, file: File) = intent(context, file)
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP)

        fun install(context: Context, file: File) =
            context.startActivity(installIntent(context, file))

    }

}
