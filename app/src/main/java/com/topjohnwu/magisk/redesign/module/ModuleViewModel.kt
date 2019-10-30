package com.topjohnwu.magisk.redesign.module

import com.topjohnwu.magisk.BR
import com.topjohnwu.magisk.model.entity.recycler.ModuleRvItem
import com.topjohnwu.magisk.redesign.compat.CompatViewModel
import com.topjohnwu.magisk.redesign.home.itemBindingOf
import com.topjohnwu.magisk.redesign.superuser.diffListOf

class ModuleViewModel : CompatViewModel() {

    val items = diffListOf<ModuleRvItem>()
    val itemBinding = itemBindingOf<ModuleRvItem> {
        it.bindExtra(BR.viewModel, this)
    }

}