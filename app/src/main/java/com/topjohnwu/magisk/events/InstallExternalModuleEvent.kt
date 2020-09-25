package com.topjohnwu.magisk.events

import android.Manifest
import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Intent
import android.widget.Toast
import androidx.annotation.RequiresPermission
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.MainDirections
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.ActivityExecutor
import com.topjohnwu.magisk.arch.BaseUIActivity
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.utils.Utils

class InstallExternalModuleEvent : ViewEvent(), ActivityExecutor {

    @RequiresPermission(allOf = [Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE])
    override fun invoke(activity: BaseUIActivity<*, *>) {
        val intent = Intent(Intent.ACTION_GET_CONTENT)
        intent.type = "application/zip"
        try {
            activity.startActivityForResult(intent, Const.ID.FETCH_ZIP)
        } catch (e: ActivityNotFoundException) {
            Utils.toast(R.string.app_not_found, Toast.LENGTH_SHORT)
        }
    }

    companion object {

        fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?): NavDirections? {
            if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
                val data = data.data
                if (data != null) {
                    return MainDirections.actionFlashFragment(data, Const.Value.FLASH_ZIP)
                }
            }
            return null
        }

    }

}
