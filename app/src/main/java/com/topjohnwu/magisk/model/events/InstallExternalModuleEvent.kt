package com.topjohnwu.magisk.model.events

import android.app.Activity
import android.content.Context
import android.content.Intent
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.legacy.flash.FlashActivity

class InstallExternalModuleEvent : ViewEvent(), ActivityExecutor {

    override fun invoke(activity: BaseActivity) {
        activity.withExternalRW {
            onSuccess {
                val intent = Intent(Intent.ACTION_GET_CONTENT)
                intent.type = "application/zip"
                activity.startActivityForResult(intent, Const.ID.FETCH_ZIP)
            }
        }
    }

    companion object {

        fun onActivityResult(context: Context, requestCode: Int, resultCode: Int, data: Intent?) {
            if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
                // Get the URI of the selected file
                val intent = context.intent<FlashActivity>()
                intent.setData(data.data).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP)
                context.startActivity(intent)
            }
        }

    }

}
