package com.topjohnwu.magisk.redesign

import android.graphics.Insets
import android.os.Bundle
import androidx.fragment.app.Fragment
import com.ncapdevi.fragnav.FragNavController
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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setSupportActionBar(binding.mainToolbar)
    }

    override fun onTabTransaction(fragment: Fragment?, index: Int) {
        super.onTabTransaction(fragment, index)

        setDisplayHomeAsUpEnabled(false)
    }

    override fun onFragmentTransaction(
        fragment: Fragment?,
        transactionType: FragNavController.TransactionType
    ) {
        super.onFragmentTransaction(fragment, transactionType)

        when (transactionType) {
            FragNavController.TransactionType.PUSH -> setDisplayHomeAsUpEnabled(true)
            else -> Unit //dunno might be useful
        }
    }

    override fun peekSystemWindowInsets(insets: Insets) {
        viewModel.insets.value = insets
    }

    fun setDisplayHomeAsUpEnabled(isEnabled: Boolean) {
        when {
            isEnabled -> binding.mainToolbar.setNavigationIcon(R.drawable.ic_back_md2)
            else -> binding.mainToolbar.navigationIcon = null
        }
    }

}