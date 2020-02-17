package com.topjohnwu.magisk.model.binding

import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.databinding.ComparableRvItem
import com.topjohnwu.magisk.model.entity.recycler.LenientRvItem
import me.tatarka.bindingcollectionadapter2.BindingRecyclerViewAdapter

class BindingAdapter <T : ComparableRvItem<*>> : BindingRecyclerViewAdapter<T>() {

    private var recyclerView: RecyclerView? = null

    override fun onBindBinding(
        binding: ViewDataBinding,
        variableId: Int,
        layoutRes: Int,
        position: Int,
        item: T
    ) {
        super.onBindBinding(binding, variableId, layoutRes, position, item)

        when (item) {
            is LenientRvItem<*> -> {
                val recycler = recyclerView ?: return
                item.onBindingBound(binding)
                item.onBindingBound(binding, recycler)
            }
            else -> item.onBindingBound(binding)
        }
    }

    override fun onAttachedToRecyclerView(recyclerView: RecyclerView) {
        super.onAttachedToRecyclerView(recyclerView)
        this.recyclerView = recyclerView
    }

}
