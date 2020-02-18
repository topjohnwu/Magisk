package com.topjohnwu.magisk.utils

import android.animation.ValueAnimator
import android.graphics.Paint
import android.graphics.drawable.Drawable
import android.view.ContextThemeWrapper
import android.view.View
import android.view.ViewGroup
import android.widget.PopupMenu
import android.widget.ProgressBar
import android.widget.TextView
import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.Toolbar
import androidx.core.view.updateLayoutParams
import androidx.databinding.BindingAdapter
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import androidx.recyclerview.widget.*
import com.google.android.material.button.MaterialButton
import com.google.android.material.card.MaterialCardView
import com.google.android.material.chip.Chip
import com.google.android.material.textfield.TextInputLayout
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.replaceRandomWithSpecial
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.superuser.internal.UiThreadHandler
import io.reactivex.Observable
import io.reactivex.disposables.Disposable
import java.util.concurrent.TimeUnit
import kotlin.math.roundToInt


@BindingAdapter("onNavigationClick")
fun setOnNavigationClickedListener(view: Toolbar, listener: View.OnClickListener) {
    view.setNavigationOnClickListener(listener)
}

@BindingAdapter("srcCompat")
fun setImageResource(view: AppCompatImageView, @DrawableRes resId: Int) {
    view.setImageResource(resId)
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

@BindingAdapter("onTouch")
fun setOnTouchListener(view: View, listener: View.OnTouchListener) {
    view.setOnTouchListener(listener)
}

@BindingAdapter("scrollToLast")
fun setScrollToLast(view: RecyclerView, shouldScrollToLast: Boolean) {

    fun scrollToLast() = UiThreadHandler.handler.postDelayed({
        view.scrollToPosition(view.adapter?.itemCount?.minus(1) ?: 0)
    }, 30)

    fun wait(callback: () -> Unit) {
        UiThreadHandler.handler.postDelayed(callback, 1000)
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

// md2

@BindingAdapter(
    "android:layout_marginLeft",
    "android:layout_marginTop",
    "android:layout_marginRight",
    "android:layout_marginBottom",
    "android:layout_marginStart",
    "android:layout_marginEnd",
    requireAll = false
)
fun View.setMargins(
    marginLeft: Int?,
    marginTop: Int?,
    marginRight: Int?,
    marginBottom: Int?,
    marginStart: Int?,
    marginEnd: Int?
) = updateLayoutParams<ViewGroup.MarginLayoutParams> {
    marginLeft?.let { leftMargin = it }
    marginTop?.let { topMargin = it }
    marginRight?.let { rightMargin = it }
    marginBottom?.let { bottomMargin = it }
    marginStart?.let { this.marginStart = it }
    marginEnd?.let { this.marginEnd = it }
}

@BindingAdapter("nestedScrollingEnabled")
fun RecyclerView.setNestedScrolling(enabled: Boolean) {
    isNestedScrollingEnabled = enabled
}

@BindingAdapter("isSelected")
fun View.isSelected(isSelected: Boolean) {
    this.isSelected = isSelected
}

@BindingAdapter("dividerVertical", "dividerHorizontal", requireAll = false)
fun RecyclerView.setDividers(dividerVertical: Drawable?, dividerHorizontal: Drawable?) {
    if (dividerHorizontal != null) {
        DividerItemDecoration(context, LinearLayoutManager.HORIZONTAL).apply {
            setDrawable(dividerHorizontal)
        }.let { addItemDecoration(it) }
    }
    if (dividerVertical != null) {
        DividerItemDecoration(context, LinearLayoutManager.VERTICAL).apply {
            setDrawable(dividerVertical)
        }.let { addItemDecoration(it) }
    }
}

@BindingAdapter("app:icon")
fun MaterialButton.setIconRes(res: Int) {
    setIconResource(res)
}

@BindingAdapter("strokeWidth")
fun MaterialCardView.setCardStrokeWidthBound(stroke: Float) {
    strokeWidth = stroke.roundToInt()
}

@BindingAdapter("onMenuClick")
fun Toolbar.setOnMenuClickListener(listener: Toolbar.OnMenuItemClickListener) {
    setOnMenuItemClickListener(listener)
}

@BindingAdapter("onCloseClicked")
fun Chip.setOnCloseClickedListenerBinding(listener: View.OnClickListener) {
    setOnCloseIconClickListener(listener)
}

@BindingAdapter("progressAnimated")
fun ProgressBar.setProgressAnimated(newProgress: Int) {
    val animator = tag as? ValueAnimator
    animator?.cancel()

    ValueAnimator.ofInt(progress, newProgress).apply {
        interpolator = FastOutSlowInInterpolator()
        addUpdateListener { progress = it.animatedValue as Int }
        tag = this
    }.start()
}

@BindingAdapter("android:text")
fun TextView.setTextSafe(text: Int) {
    if (text == 0) this.text = null else setText(text)
}

@BindingAdapter("android:onLongClick")
fun View.setOnLongClickListenerBinding(listener: () -> Unit) {
    setOnLongClickListener {
        listener()
        true
    }
}

@BindingAdapter("strikeThrough")
fun TextView.setStrikeThroughEnabled(useStrikeThrough: Boolean) {
    paintFlags = if (useStrikeThrough) {
        paintFlags or Paint.STRIKE_THRU_TEXT_FLAG
    } else {
        paintFlags and Paint.STRIKE_THRU_TEXT_FLAG.inv()
    }
}

interface OnPopupMenuItemClickListener {
    fun onMenuItemClick(itemId: Int)
}

@BindingAdapter("popupMenu", "popupMenuOnClickListener", requireAll = false)
fun View.setPopupMenu(popupMenu: Int, listener: OnPopupMenuItemClickListener) {
    val menu = tag as? PopupMenu ?: let {
        val themeWrapper = ContextThemeWrapper(context, R.style.Foundation_PopupMenu)
        PopupMenu(themeWrapper, this)
    }
    tag = menu.apply {
        this.menu.clear()
        menuInflater.inflate(popupMenu, this.menu)
        setOnMenuItemClickListener {
            listener.onMenuItemClick(it.itemId)
            true
        }
    }
    setOnClickListener {
        (tag as PopupMenu).show()
    }
}

@BindingAdapter("spanCount")
fun RecyclerView.setSpanCount(count: Int) {
    when (val lama = layoutManager) {
        is GridLayoutManager -> lama.spanCount = count
        is StaggeredGridLayoutManager -> lama.spanCount = count
    }
}
