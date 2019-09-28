package com.topjohnwu.magisk.model.entity.recycler

import androidx.databinding.ViewDataBinding
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.databinding.ComparableRvItem

/**
 * This item addresses issues where enclosing recycler has to be invalidated or generally
 * manipulated with. This shouldn't be however necessary for 99.9% of use-cases. Refrain from using
 * this item as it provides virtually no additional functionality. Stick with ComparableRvItem.
 * */
abstract class LenientRvItem<in T> : ComparableRvItem<T>() {

    open fun onBindingBound(binding: ViewDataBinding, recyclerView: RecyclerView) {}

}