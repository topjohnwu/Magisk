package com.topjohnwu.magisk.ui.module

import android.annotation.SuppressLint
import android.app.AlertDialog
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.widget.SearchView
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentReposBinding
import com.topjohnwu.magisk.model.download.DownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.model.entity.module.Repo
import com.topjohnwu.magisk.model.events.InstallModuleEvent
import com.topjohnwu.magisk.model.events.OpenChangelogEvent
import com.topjohnwu.magisk.ui.base.MagiskFragment
import com.topjohnwu.magisk.view.MarkDownWindow
import com.topjohnwu.magisk.view.dialogs.CustomAlertDialog
import org.koin.androidx.viewmodel.ext.android.sharedViewModel

class ReposFragment : MagiskFragment<ModuleViewModel, FragmentReposBinding>(),
    SearchView.OnQueryTextListener {

    override val layoutRes: Int = R.layout.fragment_repos
    override val viewModel: ModuleViewModel by sharedViewModel()

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        requireActivity().setTitle(R.string.downloads)
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is OpenChangelogEvent -> openChangelog(event.item)
            is InstallModuleEvent -> installModule(event.item)
        }
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_repo, menu)

        val query = viewModel.query.value
        val searchItem = menu.findItem(R.id.repo_search)
        val searchView = searchItem.actionView as? SearchView

        searchView?.run {
            setOnQueryTextListener(this@ReposFragment)
            setQuery(query, false)
        }

        if (query.isNotBlank()) {
            searchItem.expandActionView()
            searchView?.isIconified = false
        } else {
            searchItem.collapseActionView()
            searchView?.isIconified = true
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (item.itemId == R.id.repo_sort) {
            AlertDialog.Builder(activity)
                .setTitle(R.string.sorting_order)
                .setSingleChoiceItems(
                    R.array.sorting_orders,
                    Config.repoOrder
                ) { d, which ->
                    Config.repoOrder = which
                    viewModel.refresh(false)
                    d.dismiss()
                }.show()
        }
        return true
    }

    override fun onQueryTextSubmit(p0: String?): Boolean {
        viewModel.query.value = p0.orEmpty()
        return false
    }

    override fun onQueryTextChange(p0: String?): Boolean {
        viewModel.query.value = p0.orEmpty()
        return false
    }

    private fun openChangelog(item: Repo) {
        MarkDownWindow.show(requireActivity(), null, item.readme)
    }

    @SuppressLint("MissingPermission")
    private fun installModule(item: Repo) {
        val context = activity

        fun download(install: Boolean) = context.withExternalRW {
            onSuccess {
                DownloadService(context) {
                    val config = if (install) Configuration.Flash.Primary else Configuration.Download
                    subject = DownloadSubject.Module(item, config)
                }
            }
        }

        CustomAlertDialog(context)
            .setTitle(context.getString(R.string.repo_install_title, item.name))
            .setMessage(context.getString(R.string.repo_install_msg, item.downloadFilename))
            .setCancelable(true)
            .setPositiveButton(R.string.install) { _, _ -> download(true) }
            .setNeutralButton(R.string.download) { _, _ -> download(false) }
            .show()
    }
}
