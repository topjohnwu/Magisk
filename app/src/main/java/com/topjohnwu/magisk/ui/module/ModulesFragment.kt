package com.topjohnwu.magisk.ui.module

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.recyclerview.widget.RecyclerView
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentModulesBinding
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.model.events.OpenFilePickerEvent
import com.topjohnwu.magisk.ui.base.MagiskFragment
import com.topjohnwu.magisk.ui.flash.FlashActivity
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.sharedViewModel

class ModulesFragment : MagiskFragment<ModuleViewModel, FragmentModulesBinding>() {

    override val layoutRes: Int = R.layout.fragment_modules
    override val viewModel: ModuleViewModel by sharedViewModel()

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
            // Get the URI of the selected file
            val intent = Intent(activity, ClassMap[FlashActivity::class.java])
            intent.setData(data.data).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP)
            startActivity(intent)
        }
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.modulesContent.addOnScrollListener(object : RecyclerView.OnScrollListener() {
            override fun onScrolled(recyclerView: RecyclerView, dx: Int, dy: Int) {
                binding.modulesRefreshLayout.isEnabled = recyclerView.getChildAt(0).top >= 0
            }
        })
    }

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is OpenFilePickerEvent -> selectFile()
        }
    }

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        requireActivity().setTitle(R.string.modules)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_reboot, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.reboot -> {
                reboot()
                return true
            }
            R.id.reboot_recovery -> {
                Shell.su("/system/bin/reboot recovery").submit()
                return true
            }
            R.id.reboot_bootloader -> {
                reboot("bootloader")
                return true
            }
            R.id.reboot_download -> {
                reboot("download")
                return true
            }
            R.id.reboot_edl -> {
                reboot("edl")
                return true
            }
            else -> return false
        }
    }

    private fun selectFile() {
        activity.withExternalRW {
            onSuccess {
                val intent = Intent(Intent.ACTION_GET_CONTENT)
                intent.type = "application/zip"
                startActivityForResult(intent, Const.ID.FETCH_ZIP)
            }
        }
    }
}
