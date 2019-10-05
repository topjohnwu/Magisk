package com.topjohnwu.magisk.model.events

import android.content.Context
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment

interface ContextExecutor {

    operator fun invoke(context: Context)

}

interface ActivityExecutor {

    operator fun invoke(activity: AppCompatActivity)

}

interface FragmentExecutor {

    operator fun invoke(fragment: Fragment)

}