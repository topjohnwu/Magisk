package com.topjohnwu.magisk.model.events

import android.content.Context
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.core.base.BaseActivity

interface ContextExecutor {

    operator fun invoke(context: Context)

}

interface ActivityExecutor {

    operator fun invoke(activity: BaseActivity)

}

interface FragmentExecutor {

    operator fun invoke(fragment: Fragment)

}
