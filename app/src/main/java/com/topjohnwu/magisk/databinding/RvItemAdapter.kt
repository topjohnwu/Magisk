package com.topjohnwu.magisk.databinding

import android.annotation.SuppressLint
import android.util.SparseArray
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.databinding.BindingAdapter
import androidx.databinding.DataBindingUtil
import androidx.databinding.ObservableList
import androidx.databinding.ObservableList.OnListChangedCallback
import androidx.databinding.ViewDataBinding
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.findViewTreeLifecycleOwner
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.BR

class RvItemAdapter<T: RvItem>(
    val items: List<T>,
    val extraBindings: SparseArray<*>?
) : RecyclerView.Adapter<RvItemAdapter.ViewHolder>() {

    private var lifecycleOwner: LifecycleOwner? = null
    private var recyclerView: RecyclerView? = null
    private val observer by lazy(LazyThreadSafetyMode.NONE) { ListObserver<T>() }

    override fun onAttachedToRecyclerView(rv: RecyclerView) {
        lifecycleOwner = rv.findViewTreeLifecycleOwner()
        recyclerView = rv
        if (items is ObservableList)
            items.addOnListChangedCallback(observer)
    }

    override fun onDetachedFromRecyclerView(rv: RecyclerView) {
        lifecycleOwner = null
        recyclerView = null
        if (items is ObservableList)
            items.removeOnListChangedCallback(observer)
    }

    override fun onCreateViewHolder(parent: ViewGroup, layoutRes: Int): ViewHolder {
        val inflator = LayoutInflater.from(parent.context)
        return ViewHolder(DataBindingUtil.inflate(inflator, layoutRes, parent, false))
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val item = items[position]
        holder.binding.setVariable(BR.item, item)
        extraBindings?.let {
            for (i in 0 until it.size()) {
                holder.binding.setVariable(it.keyAt(i), it.valueAt(i))
            }
        }
        holder.binding.lifecycleOwner = lifecycleOwner
        holder.binding.executePendingBindings()
        recyclerView?.let {
            if (item is ViewAwareItem)
                item.onBind(holder.binding, it)
        }
    }

    override fun getItemCount() = items.size

    override fun getItemViewType(position: Int) = items[position].layoutRes

    class ViewHolder(val binding: ViewDataBinding) : RecyclerView.ViewHolder(binding.root)

    inner class ListObserver<T: RvItem> : OnListChangedCallback<ObservableList<T>>() {

        @SuppressLint("NotifyDataSetChanged")
        override fun onChanged(sender: ObservableList<T>) {
            notifyDataSetChanged()
        }

        override fun onItemRangeChanged(
            sender: ObservableList<T>,
            positionStart: Int,
            itemCount: Int
        ) {
            notifyItemRangeChanged(positionStart, itemCount)
        }

        override fun onItemRangeInserted(
            sender: ObservableList<T>?,
            positionStart: Int,
            itemCount: Int
        ) {
            notifyItemRangeInserted(positionStart, itemCount)
        }

        override fun onItemRangeMoved(
            sender: ObservableList<T>?,
            fromPosition: Int,
            toPosition: Int,
            itemCount: Int
        ) {
            for (i in 0 until itemCount) {
                notifyItemMoved(fromPosition + i, toPosition + i)
            }
        }

        override fun onItemRangeRemoved(
            sender: ObservableList<T>?,
            positionStart: Int,
            itemCount: Int
        ) {
            notifyItemRangeRemoved(positionStart, itemCount)
        }
    }
}

inline fun bindExtra(body: (SparseArray<Any?>) -> Unit) = SparseArray<Any?>().also(body)

@BindingAdapter("items", "extraBindings", requireAll = false)
fun <T: RvItem> RecyclerView.setAdapter(items: List<T>?, extraBindings: SparseArray<*>?) {
    if (items != null) {
        val rva = (adapter as? RvItemAdapter<*>)
        if (rva == null || rva.items !== items || rva.extraBindings !== extraBindings) {
            adapter = RvItemAdapter(items, extraBindings)
        }
    }
}
