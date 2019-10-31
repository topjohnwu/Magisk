package com.topjohnwu.magisk.redesign.hide

import android.content.Context
import android.graphics.Insets
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHideMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class HideFragment : CompatFragment<HideViewModel, FragmentHideMd2Binding>() {

    override val layoutRes = R.layout.fragment_hide_md2
    override val viewModel by viewModel<HideViewModel>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onAttach(context: Context) {
        super.onAttach(context)
        activity.setTitle(R.string.magiskhide)
        setHasOptionsMenu(true)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_hide_md2, menu)
        menu.findItem(R.id.action_show_system)?.isChecked = viewModel.isShowSystem
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return viewModel.menuItemPressed(item)
    }

}