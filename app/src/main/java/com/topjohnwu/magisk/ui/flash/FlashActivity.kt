package com.topjohnwu.magisk.ui.flash

import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityFlashBinding
import com.topjohnwu.magisk.ui.base.MagiskActivity
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.koin.core.parameter.parametersOf

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

}
