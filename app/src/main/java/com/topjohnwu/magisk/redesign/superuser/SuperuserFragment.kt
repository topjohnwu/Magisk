package com.topjohnwu.magisk.redesign.superuser

import android.graphics.Insets
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSuperuserMd2Binding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class SuperuserFragment : CompatFragment<SuperuserViewModel, FragmentSuperuserMd2Binding>() {

    override val layoutRes = R.layout.fragment_superuser_md2
    override val viewModel by viewModel<SuperuserViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.section_superuser)
        setHasOptionsMenu(true)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_superuser_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem) = when (item.itemId) {
        R.id.action_log -> Navigation.log().dispatchOnSelf()
        else -> null
    }?.let { true } ?: super.onOptionsItemSelected(item)

}