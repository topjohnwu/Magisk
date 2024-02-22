package com.topjohnwu.magisk.ui.deny

import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.appcompat.widget.SearchView
import androidx.core.view.MenuProvider
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.core.ktx.hideKeyboard
import com.topjohnwu.magisk.databinding.FragmentDenyMd2Binding
import rikka.recyclerview.addEdgeSpacing
import rikka.recyclerview.addItemSpacing
import rikka.recyclerview.fixEdgeEffect
import com.topjohnwu.magisk.core.Info

class DenyListFragment : BaseFragment<FragmentDenyMd2Binding>(), MenuProvider {

    override val layoutRes = R.layout.fragment_deny_md2
    override val viewModel by viewModel<DenyListViewModel>()

    private lateinit var searchView: SearchView

    override fun onStart() {
        super.onStart()
        if (Info.sulist) activity?.setTitle(R.string.settings_sulist)
        else activity?.setTitle(R.string.settings_magiskhide_title)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.appList.addOnScrollListener(object : RecyclerView.OnScrollListener() {
            override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {
                if (newState != RecyclerView.SCROLL_STATE_IDLE) activity?.hideKeyboard()
            }
        })

        binding.appList.apply {
            addEdgeSpacing(top = R.dimen.l_50, bottom = R.dimen.l1)
            addItemSpacing(R.dimen.l1, R.dimen.l_50, R.dimen.l1)
            fixEdgeEffect()
        }
    }

    override fun onPreBind(binding: FragmentDenyMd2Binding) = Unit

    override fun onBackPressed(): Boolean {
        if (searchView.isIconfiedByDefault && !searchView.isIconified) {
            searchView.isIconified = true
            return true
        }
        return super.onBackPressed()
    }

    override fun onCreateMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_deny_md2, menu)
        searchView = menu.findItem(R.id.action_search).actionView as SearchView
        searchView.queryHint = searchView.context.getString(R.string.hide_filter_hint)
        searchView.setOnQueryTextListener(object : SearchView.OnQueryTextListener {
            override fun onQueryTextSubmit(query: String?): Boolean {
                viewModel.query = query ?: ""
                return true
            }

            override fun onQueryTextChange(newText: String?): Boolean {
                viewModel.query = newText ?: ""
                return true
            }
        })
    }

    override fun onMenuItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_show_system -> {
                val check = !item.isChecked
                viewModel.isShowSystem = check
                item.isChecked = check
                return true
            }
            R.id.action_show_OS -> {
                val check = !item.isChecked
                viewModel.isShowOS = check
                item.isChecked = check
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onPrepareMenu(menu: Menu) {
        val showSystem = menu.findItem(R.id.action_show_system)
        val showOS = menu.findItem(R.id.action_show_OS)
        showOS.isEnabled = showSystem.isChecked
    }
}
