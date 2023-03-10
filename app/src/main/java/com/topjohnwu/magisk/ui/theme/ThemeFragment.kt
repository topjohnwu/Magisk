package com.topjohnwu.magisk.ui.theme

import android.os.Build
import android.os.Bundle
import android.view.*
import android.widget.FrameLayout
import android.widget.LinearLayout
import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.databinding.FragmentThemeMd2Binding
import com.topjohnwu.magisk.databinding.ItemThemeBindingImpl
import com.topjohnwu.magisk.events.dialog.DarkThemeDialog

class ThemeFragment : BaseFragment<FragmentThemeMd2Binding>() {

    override val layoutRes = R.layout.fragment_theme_md2
    override val viewModel by viewModel<ThemeViewModel>()

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity?.title = getString(R.string.section_theme)
    }
    override fun onCreateView(
            inflater: LayoutInflater,
            container: ViewGroup?,
            savedInstanceState: Bundle?
    ): View {
        super.onCreateView(inflater, container, savedInstanceState)

        val view = binding.root.findViewById<LinearLayout>(R.id.theme_container)

        for(a in Theme.values()){
            if(Build.VERSION.SDK_INT < Build.VERSION_CODES.S && a == Theme.Dynamic)
                continue
            val themed = ContextThemeWrapper(activity, a.themeRes)
            ItemThemeBindingImpl.inflate(LayoutInflater.from(themed), view, true).also {
                it.setVariable(BR.viewModel, viewModel)
                it.setVariable(BR.theme, a)
                it.lifecycleOwner = viewLifecycleOwner
            }

        }

        return binding.root
    }
    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_theme, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when(item.itemId){
            R.id.action_theme_mode -> viewModel.onThemeModeOptionClicked()
        }
        return super.onOptionsItemSelected(item)
    }
}