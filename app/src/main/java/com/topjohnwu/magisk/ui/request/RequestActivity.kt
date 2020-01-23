package com.topjohnwu.magisk.ui.request

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.ActivityRequestMd2Binding
import com.topjohnwu.magisk.ui.base.CompatActivity
import org.koin.androidx.viewmodel.ext.android.viewModel

class RequestActivity : CompatActivity<RequestViewModel, ActivityRequestMd2Binding>() {

    override val navHost = TODO()
    override val layoutRes = R.layout.activity_request_md2
    override val viewModel by viewModel<RequestViewModel>()

}
