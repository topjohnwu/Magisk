package com.topjohnwu.magisk.ui.home

import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class HomeFragment : BaseUIFragment<HomeViewModel, FragmentHomeMd2Binding>() {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.section_home)
        setHasOptionsMenu(true)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_home_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem) = when (item.itemId) {
        R.id.action_settings -> Navigation.settings().dispatchOnSelf()
        else -> null
    }?.let { true } ?: super.onOptionsItemSelected(item)

}
