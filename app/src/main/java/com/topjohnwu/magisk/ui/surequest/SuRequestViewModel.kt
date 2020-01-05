package com.topjohnwu.magisk.ui.surequest

import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.content.res.Resources
import android.graphics.drawable.Drawable
import android.os.CountDownTimer
import com.topjohnwu.magisk.BuildConfig
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.base.viewmodel.BaseViewModel
import com.topjohnwu.magisk.data.database.PolicyDao
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.extensions.now
import com.topjohnwu.magisk.model.entity.MagiskPolicy
import com.topjohnwu.magisk.model.entity.recycler.SpinnerRvItem
import com.topjohnwu.magisk.model.entity.toPolicy
import com.topjohnwu.magisk.model.events.DieEvent
import com.topjohnwu.magisk.utils.BiometricHelper
import com.topjohnwu.magisk.utils.DiffObservableList
import com.topjohnwu.magisk.utils.KObservableField
import com.topjohnwu.magisk.utils.SuConnector
import me.tatarka.bindingcollectionadapter2.BindingListViewAdapter
import me.tatarka.bindingcollectionadapter2.ItemBinding
import timber.log.Timber
import java.io.IOException
import java.util.concurrent.TimeUnit.*

class SuRequestViewModel(
    private val packageManager: PackageManager,
    private val policyDB: PolicyDao,
    private val timeoutPrefs: SharedPreferences,
    private val resources: Resources
) : BaseViewModel() {

    val icon = KObservableField<Drawable?>(null)
    val title = KObservableField("")
    val packageName = KObservableField("")

    val denyText = KObservableField(resources.getString(R.string.deny))
    val warningText = KObservableField<CharSequence>(resources.getString(R.string.su_warning))

    val selectedItemPosition = KObservableField(0)

    private val items = DiffObservableList(ComparableRvItem.callback)
    private val itemBinding = ItemBinding.of<ComparableRvItem<*>> { binding, _, item ->
        item.bind(binding)
    }

    val adapter = BindingListViewAdapter<ComparableRvItem<*>>(1).apply {
        itemBinding = this@SuRequestViewModel.itemBinding
        setItems(items)
    }

    private val cancelTasks = mutableListOf<() -> Unit>()

    private lateinit var timer: CountDownTimer
    private lateinit var policy: MagiskPolicy
    private lateinit var connector: SuConnector

    private fun cancelTimer() {
        timer.cancel()
        denyText.value = resources.getString(R.string.deny)
    }

    fun grantPressed() {
        cancelTimer()
        if (BiometricHelper.isEnabled) {
            withView {
                BiometricHelper.authenticate(this) {
                    handleAction(MagiskPolicy.ALLOW)
                }
            }
        } else {
            handleAction(MagiskPolicy.ALLOW)
        }
    }

    fun denyPressed() {
        handleAction(MagiskPolicy.DENY)
        timer.cancel()
    }

    fun spinnerTouched(): Boolean {
        cancelTimer()
        return false
    }

    fun handleRequest(intent: Intent): Boolean {
        val socketName = intent.getStringExtra("socket") ?: return false

        try {
            connector = Connector(socketName)
            val map = connector.readRequest()
            val uid = map["uid"]?.toIntOrNull() ?: return false
            policy = uid.toPolicy(packageManager)
        } catch (e: Exception) {
            Timber.e(e)
            return false
        }

        // Never allow com.topjohnwu.magisk (could be malware)
        if (policy.packageName == BuildConfig.APPLICATION_ID)
            return false

        when (Config.suAutoReponse) {
            Config.Value.SU_AUTO_DENY -> {
                handleAction(MagiskPolicy.DENY, 0)
                return true
            }
            Config.Value.SU_AUTO_ALLOW -> {
                handleAction(MagiskPolicy.ALLOW, 0)
                return true
            }
        }

        showUI()
        return true
    }

    private fun showUI() {
        resources.getStringArray(R.array.allow_timeout)
            .map { SpinnerRvItem(it) }
            .let { items.update(it) }

        icon.value = policy.applicationInfo.loadIcon(packageManager)
        title.value = policy.appName
        packageName.value = policy.packageName
        selectedItemPosition.value = timeoutPrefs.getInt(policy.packageName, 0)

        val millis = SECONDS.toMillis(Config.suDefaultTimeout.toLong())
        timer = object : CountDownTimer(millis, 1000) {
            override fun onTick(remains: Long) {
                denyText.value = "${resources.getString(R.string.deny)} (${remains / 1000})"
            }

            override fun onFinish() {
                denyText.value = resources.getString(R.string.deny)
                handleAction(MagiskPolicy.DENY)
            }
        }
        timer.start()
        cancelTasks.add { cancelTimer() }
    }

    private fun handleAction() {
        connector.response()
        cancelTasks.forEach { it() }
        DieEvent().publish()
    }

    private fun handleAction(action: Int) {
        val pos = selectedItemPosition.value
        timeoutPrefs.edit().putInt(policy.packageName, pos).apply()
        handleAction(action, Config.Value.TIMEOUT_LIST[pos])
    }

    private fun handleAction(action: Int, time: Int) {
        val until = if (time > 0)
            MILLISECONDS.toSeconds(now) + MINUTES.toSeconds(time.toLong())
        else
            time.toLong()

        policy.policy = action
        policy.until = until
        policy.uid = policy.uid % 100000 + Const.USER_ID * 100000

        if (until >= 0)
            policyDB.update(policy).blockingAwait()

        handleAction()
    }

    private inner class Connector @Throws(Exception::class)
    internal constructor(name: String) : SuConnector(name) {
        @Throws(IOException::class)
        override fun onResponse() {
            out.writeInt(policy.policy)
        }
    }

}
