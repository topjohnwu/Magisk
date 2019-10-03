package com.topjohnwu.magisk.redesign

import android.graphics.Insets
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatActivity
import org.koin.androidx.viewmodel.ext.android.viewModel

open class MainActivity : CompatActivity<MainViewModel, ActivityMainMd2Binding>() {

    override val layoutRes = R.layout.activity_main_md2
    override val viewModel by viewModel<MainViewModel>()

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

}