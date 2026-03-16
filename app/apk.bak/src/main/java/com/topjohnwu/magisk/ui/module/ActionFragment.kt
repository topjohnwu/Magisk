package com.topjohnwu.magisk.ui.module

import android.annotation.SuppressLint
import android.content.pm.ActivityInfo
import android.os.Bundle
import android.view.KeyEvent
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewTreeObserver
import android.widget.Toast
import androidx.core.view.MenuProvider
import androidx.core.view.isVisible
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.databinding.FragmentActionMd2Binding
import com.topjohnwu.magisk.core.R as CoreR

class ActionFragment : BaseFragment<FragmentActionMd2Binding>(), MenuProvider {

    override val layoutRes = R.layout.fragment_action_md2
    override val viewModel by viewModel<ActionViewModel>()
    override val snackbarView: View get() = binding.snackbarContainer

    private var defaultOrientation = -1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewModel.args = ActionFragmentArgs.fromBundle(requireArguments())
    }

    override fun onStart() {
        super.onStart()
        activity?.setTitle(viewModel.args.name)
        binding.closeBtn.setOnClickListener {
            activity?.onBackPressed()
        }

        viewModel.state.observe(this) {
            if (it != ActionViewModel.State.RUNNING) {
                binding.closeBtn.apply {
                    if (!this.isVisible) this.show()
                    if (!this.isFocused) this.requestFocus()
                }
            }
            if (it != ActionViewModel.State.SUCCESS) return@observe
            view?.viewTreeObserver?.addOnWindowFocusChangeListener(
                object : ViewTreeObserver.OnWindowFocusChangeListener {
                    override fun onWindowFocusChanged(hasFocus: Boolean) {
                        if (hasFocus) return
                        view?.viewTreeObserver?.removeOnWindowFocusChangeListener(this)
                        view?.context?.apply {
                            toast(
                                getString(CoreR.string.done_action, viewModel.args.name),
                                Toast.LENGTH_SHORT
                            )
                        }
                        viewModel.back()
                    }
                }
            )
        }
    }

    override fun onCreateMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_flash, menu)
    }

    override fun onMenuItemSelected(item: MenuItem): Boolean {
        return viewModel.onMenuItemClicked(item)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        defaultOrientation = activity?.requestedOrientation ?: -1
        activity?.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
        if (savedInstanceState == null) {
            viewModel.startRunAction()
        }
    }

    @SuppressLint("WrongConstant")
    override fun onDestroyView() {
        if (defaultOrientation != -1) {
            activity?.requestedOrientation = defaultOrientation
        }
        super.onDestroyView()
    }

    override fun onKeyEvent(event: KeyEvent): Boolean {
        return when (event.keyCode) {
            KeyEvent.KEYCODE_VOLUME_UP, KeyEvent.KEYCODE_VOLUME_DOWN -> true

            else -> false
        }
    }

    override fun onBackPressed(): Boolean {
        if (viewModel.state.value == ActionViewModel.State.RUNNING) return true
        return super.onBackPressed()
    }

    override fun onPreBind(binding: FragmentActionMd2Binding) = Unit
}
