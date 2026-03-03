package com.topjohnwu.magisk.ui.home

import android.os.Bundle
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.ViewCompositionStrategy
import androidx.core.view.MenuProvider
import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.findNavController
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.NavigationActivity
import com.topjohnwu.magisk.arch.VMFactory
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadEngine
import com.topjohnwu.magisk.ui.theme.MagiskTheme
import com.topjohnwu.magisk.core.R as CoreR

class HomeFragment : Fragment(), MenuProvider {

    private val viewModel by lazy {
        ViewModelProvider(this, VMFactory)[HomeViewModel::class.java]
    }

    override fun onStart() {
        super.onStart()
        (activity as? NavigationActivity<*>)?.setTitle(CoreR.string.section_home)
        DownloadEngine.observeProgress(this, viewModel::onProgressUpdate)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return ComposeView(requireContext()).apply {
            setViewCompositionStrategy(ViewCompositionStrategy.DisposeOnViewTreeLifecycleDestroyed)
            setContent {
                MagiskTheme {
                    HomeScreen(viewModel = viewModel)
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        activity?.addMenuProvider(this, viewLifecycleOwner, Lifecycle.State.STARTED)
    }

    override fun onCreateMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_home_md2, menu)
        if (!Info.isRooted)
            menu.removeItem(R.id.action_reboot)
    }

    override fun onMenuItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_settings ->
                activity?.let {
                    NavigationActivity.navigate(
                        HomeFragmentDirections.actionHomeFragmentToSettingsFragment(),
                        it.findNavController(R.id.main_nav_host),
                        it.contentResolver,
                    )
                }
            R.id.action_reboot -> (activity as? NavigationActivity<*>)?.let {
                RebootMenu.inflate(it).show()
            }
            else -> return false
        }
        return true
    }

    override fun onResume() {
        super.onResume()
        viewModel.resetProgress()
    }
}
