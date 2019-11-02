package com.topjohnwu.magisk.utils

import android.view.View
import android.widget.TextView
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.Toolbar
import androidx.databinding.BindingAdapter
import androidx.databinding.InverseBindingAdapter
import androidx.databinding.InverseBindingListener
import androidx.drawerlayout.widget.DrawerLayout
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager.widget.ViewPager
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.navigation.NavigationView
import com.google.android.material.textfield.TextInputLayout
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.replaceRandomWithSpecial
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.state.IndeterminateState
import io.reactivex.Observable
import io.reactivex.disposables.Disposable
import java.util.concurrent.TimeUnit


@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(view: Toolbar, listener: View.OnClickListener) {
    view.setNavigationOnClickListener(listener)
}

@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(
    view: NavigationView,
    listener: NavigationView.OnNavigationItemSelectedListener
) {
    view.setNavigationItemSelectedListener {
        (view.parent as? DrawerLayout)?.closeDrawers()
        listener.onNavigationItemSelected(it)
    }
}

@BindingAdapter("srcCompat")
fun setImageResource(view: AppCompatImageView, @DrawableRes resId: Int) {
    view.setImageResource(resId)
}

@BindingAdapter("app:tint")
fun setTint(view: AppCompatImageView, @ColorInt tint: Int) {
    view.setColorFilter(tint)
}

@BindingAdapter("isChecked")
fun setChecked(view: AppCompatImageView, isChecked: Boolean) {
    val state = when (isChecked) {
        true -> IndeterminateState.CHECKED
        else -> IndeterminateState.UNCHECKED
    }
    setChecked(view, state)
}

@BindingAdapter("isChecked")
fun setChecked(view: AppCompatImageView, isChecked: IndeterminateState) {
    view.setImageResource(
        when (isChecked) {
            IndeterminateState.INDETERMINATE -> R.drawable.ic_indeterminate
            IndeterminateState.CHECKED -> R.drawable.ic_checked
            IndeterminateState.UNCHECKED -> R.drawable.ic_unchecked
        }
    )
}

@BindingAdapter("position")
fun setPosition(view: ViewPager, position: Int) {
    view.currentItem = position
}

@InverseBindingAdapter(attribute = "position", event = "positionChanged")
fun getPosition(view: ViewPager) = view.currentItem

@BindingAdapter("positionChanged")
fun setPositionChangedListener(view: ViewPager, listener: InverseBindingListener) {
    view.addOnPageChangeListener(object : ViewPager.OnPageChangeListener {
        override fun onPageSelected(position: Int) = listener.onChange()
        override fun onPageScrollStateChanged(state: Int) = listener.onChange()
        override fun onPageScrolled(
            position: Int,
            positionOffset: Float,
            positionOffsetPixels: Int
        ) = listener.onChange()
    })
}

@BindingAdapter("invisibleScale")
fun setInvisibleWithScale(view: View, isInvisible: Boolean) {
    view.animate()
        .scaleX(if (isInvisible) 0f else 1f)
        .scaleY(if (isInvisible) 0f else 1f)
        .setInterpolator(FastOutSlowInInterpolator())
        .start()
}

@BindingAdapter("movieBehavior", "movieBehaviorText")
fun setMovieBehavior(view: TextView, isMovieBehavior: Boolean, text: String) {
    (view.tag as? Disposable)?.dispose()
    if (isMovieBehavior) {
        val observer = Observable
            .interval(150, TimeUnit.MILLISECONDS)
            .subscribeK {
                view.text = text.replaceRandomWithSpecial()
            }
        view.tag = observer
    } else {
        view.text = text
    }
}

/*@BindingAdapter("selection"*//*, "selectionAttrChanged", "adapter"*//*)
fun setSelectedItemPosition(view: Spinner, position: Int) {
    view.setSelection(position)
}

@InverseBindingAdapter(
    attribute = "android:selectedItemPosition",
    event = "android:selectedItemPositionAttrChanged"
)
fun getSelectedItemPosition(view: Spinner) = view.selectedItemPosition

@BindingAdapter("selectedItemPositionAttrChanged")
fun setSelectedItemPositionListener(view: Spinner, listener: InverseBindingListener) {
    view.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
        override fun onNothingSelected(p0: AdapterView<*>?) {
            listener.onChange()
        }

        override fun onItemSelected(p0: AdapterView<*>?, p1: View?, p2: Int, p3: Long) {
            listener.onChange()
        }
    }
}*/

@BindingAdapter("onTouch")
fun setOnTouchListener(view: View, listener: View.OnTouchListener) {
    view.setOnTouchListener(listener)
}

@BindingAdapter("scrollToLast")
fun setScrollToLast(view: RecyclerView, shouldScrollToLast: Boolean) {

    fun scrollToLast() = view.post {
        view.scrollToPosition(view.adapter?.itemCount?.minus(1) ?: 0)
    }

    fun wait(callback: () -> Unit) {
        Observable.timer(1, TimeUnit.SECONDS).subscribeK { callback() }
    }

    fun RecyclerView.Adapter<*>.setListener() {
        val observer = object : RecyclerView.AdapterDataObserver() {
            override fun onItemRangeInserted(positionStart: Int, itemCount: Int) {
                scrollToLast()
            }
        }
        registerAdapterDataObserver(observer)
        view.setTag(R.id.recyclerScrollListener, observer)
    }

    fun RecyclerView.Adapter<*>.removeListener() {
        val observer =
            view.getTag(R.id.recyclerScrollListener) as? RecyclerView.AdapterDataObserver ?: return
        unregisterAdapterDataObserver(observer)
    }

    fun trySetListener(): Unit = view.adapter?.setListener() ?: wait { trySetListener() }

    if (shouldScrollToLast) {
        trySetListener()
    } else {
        view.adapter?.removeListener()
    }
}

@BindingAdapter("hide")
fun setHidden(view: FloatingActionButton, hide: Boolean) {
    if (hide) view.hide() else view.show()
}

@BindingAdapter("scrollPosition", "scrollPositionSmooth", requireAll = false)
fun setScrollPosition(view: RecyclerView, position: Int, smoothScroll: Boolean) {
    val adapterItemCount = view.adapter?.itemCount ?: -1
    if (position !in 0 until adapterItemCount) {
        // the position is not in adapter bounds, adapter will throw exception for invalid positions
        return
    }

    when {
        smoothScroll -> view.smoothScrollToPosition(position)
        else -> view.scrollToPosition(position)
    }
}

@BindingAdapter("recyclerScrollEvent")
fun setScrollListener(view: RecyclerView, listener: InverseBindingListener) {
    view.addOnScrollListener(object : RecyclerView.OnScrollListener() {
        override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {
            // don't change this or the recycler will stop at every line, effectively disabling smooth scroll
            if (newState == RecyclerView.SCROLL_STATE_IDLE) {
                listener.onChange()
            }
        }
    })
}

@InverseBindingAdapter(attribute = "scrollPosition", event = "recyclerScrollEvent")
fun getScrollPosition(view: RecyclerView) = (view.layoutManager as? LinearLayoutManager)
    ?.findLastCompletelyVisibleItemPosition()
    ?: -1

@BindingAdapter("isEnabled")
fun setEnabled(view: View, isEnabled: Boolean) {
    view.isEnabled = isEnabled
}

@BindingAdapter("error")
fun TextInputLayout.setErrorString(error: String) {
    val newError = error.let { if (it.isEmpty()) null else it }
    if (this.error == null && newError == null) return
    this.error = newError
}