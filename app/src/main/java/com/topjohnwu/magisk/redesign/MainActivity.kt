package com.topjohnwu.magisk.redesign

import android.graphics.Insets
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityMainMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatActivity
import com.topjohnwu.magisk.redesign.home.HomeFragment
import org.koin.androidx.viewmodel.ext.android.viewModel
import kotlin.reflect.KClass

open class MainActivity : CompatActivity<MainViewModel, ActivityMainMd2Binding>() {

    override val layoutRes = R.layout.activity_main_md2
    override val viewModel by viewModel<MainViewModel>()
    override val navHostId: Int = R.id.main_nav_host
    override val defaultPosition: Int = 0

    override val baseFragments: List<KClass<out Fragment>> = listOf(
        HomeFragment::class
    )

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

}