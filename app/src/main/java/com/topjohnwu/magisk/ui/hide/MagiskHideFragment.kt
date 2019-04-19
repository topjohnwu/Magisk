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
            (findItem(R.id.app_search).actionView as? SearchView)
                ?.setOnQueryTextListener(this@MagiskHideFragment)

            val showSystem = Config.get<Boolean>(Config.Key.SHOW_SYSTEM_APP)

            findItem(R.id.show_system).isChecked = showSystem
            viewModel.isShowSystem.value = showSystem
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == R.id.show_system) {
            val showSystem = !item.isChecked
            item.isChecked = showSystem
            Config.set(Config.Key.SHOW_SYSTEM_APP, showSystem)
            viewModel.isShowSystem.value = showSystem
            //adapter!!.setShowSystem(showSystem)
            //adapter!!.filter(search!!.query.toString())
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

    /*override fun onEvent(event: Int) {
        //mSwipeRefreshLayout!!.isRefreshing = false
        adapter!!.filter(search!!.query.toString())
    }*/
}
