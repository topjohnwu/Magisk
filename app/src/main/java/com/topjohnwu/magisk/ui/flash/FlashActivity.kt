package com.topjohnwu.magisk.ui.flash

import android.content.Context
import android.content.Intent
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
        val uri = intent.data
        val action = intent.getStringExtra(Const.Key.FLASH_ACTION) ?: let { finish();"" }
        parametersOf(action, uri)
    }

    override fun onBackPressed() {
        if (viewModel.loading) return
        super.onBackPressed()
    }

    companion object {

        private fun intent(context: Context) = Intent(context, ClassMap[FlashActivity::class.java])

        fun flashMagiskIntent(context: Context, file: File) = intent(context)
            .setData(file.toUri())
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK)

        fun flashMagisk(context: Context, file: File) =
            context.startActivity(flashMagiskIntent(context, file))


        fun flashModuleIntent(context: Context, file: File) = intent(context)
            .setData(file.toUri())
            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP)

        fun flashModule(context: Context, file: File) =
            context.startActivity(flashModuleIntent(context, file))

    }

}
