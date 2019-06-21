package com.topjohnwu.magisk.ui.hide

import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.widget.SearchView
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentMagiskHideBinding
import com.topjohnwu.magisk.ui.base.MagiskFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class MagiskHideFragment : MagiskFragment<HideViewModel, FragmentMagiskHideBinding>(),
    SearchView.OnQueryTextListener {

    override val layoutRes: Int = R.layout.fragment_magisk_hide
    override val viewModel: HideViewModel by viewModel()

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        requireActivity().setTitle(R.string.magiskhide)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_magiskhide, menu)
        menu.apply {
            val query = viewModel.query.value
            val searchItem = menu.findItem(R.id.app_search)
            val searchView = searchItem.actionView as? SearchView

            searchView?.run {
                setOnQueryTextListener(this@MagiskHideFragment)
                setQuery(query, false)
            }

            if (query.isNotBlank()) {
                searchItem.expandActionView()
                searchView?.isIconified = false
            } else {
                searchItem.collapseActionView()
                searchView?.isIconified = true
            }

            val showSystem = Config.showSystemApp

            findItem(R.id.show_system).isChecked = showSystem
            viewModel.isShowSystem.value = showSystem
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == R.id.show_system) {
            val showSystem = !item.isChecked
            item.isChecked = showSystem
            Config.showSystemApp = showSystem
            viewModel.isShowSystem.value = showSystem
        }
        return true
    }

    override fun onQueryTextSubmit(query: String?): Boolean {
        viewModel.query.value = query.orEmpty()
        return false
    }

    override fun onQueryTextChange(query: String?): Boolean {
        viewModel.query.value = query.orEmpty()
        return false
    }
}
