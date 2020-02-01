package com.topjohnwu.magisk.utils

import android.animation.Animator
import android.animation.ValueAnimator
import android.graphics.Paint
import android.graphics.drawable.Drawable
import android.os.Build
import android.view.ContextThemeWrapper
import android.view.View
import android.view.ViewAnimationUtils
import android.view.ViewGroup
import android.widget.PopupMenu
import android.widget.ProgressBar
import android.widget.TextView
import androidx.annotation.ColorInt
import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.Toolbar
import androidx.core.animation.doOnEnd
import androidx.core.view.*
import androidx.databinding.BindingAdapter
import androidx.databinding.InverseBindingAdapter
import androidx.databinding.InverseBindingListener
import androidx.drawerlayout.widget.DrawerLayout
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import androidx.recyclerview.widget.DividerItemDecoration
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager.widget.ViewPager
import com.google.android.material.button.MaterialButton
import com.google.android.material.card.MaterialCardView
import com.google.android.material.chip.Chip
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.google.android.material.navigation.NavigationView
import com.google.android.material.textfield.TextInputLayout
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.drawableCompat
import com.topjohnwu.magisk.extensions.replaceRandomWithSpecial
import com.topjohnwu.magisk.extensions.subscribeK
import com.topjohnwu.magisk.model.entity.state.IndeterminateState
import io.reactivex.Observable
import io.reactivex.disposables.Disposable
import java.util.concurrent.TimeUnit
import kotlin.math.hypot
import kotlin.math.roundToInt


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

// md2

@BindingAdapter("onSelectClick", "onSelectReset", requireAll = false)
fun View.setOnSelectClickListener(listener: View.OnClickListener, resetTime: Long) {

    fun getHideTarget() = (parent as? ViewGroup)?.findViewWithTag<View>(R.id.hideWhenSelected)
    fun animateVisibility(hide: Boolean, target: View? = getHideTarget()) {
        target ?: return
        val targetScale = if (hide) 0f else 1f
        target.animate()
            .scaleY(targetScale)
            .scaleX(targetScale)
            .start()
    }

    setOnClickListener {
        when {
            it.isSelected -> {
                animateVisibility(false)
                listener.onClick(it)
                (it.tag as? Runnable)?.let { task ->
                    it.handler.removeCallbacks(task)
                }
                it.isSelected = false
            }
            else -> {
                animateVisibility(true)
                it.isSelected = true
                it.tag = it.postDelayed(resetTime) {
                    animateVisibility(false)
                    it.tag = null
                    it.isSelected = false
                }
            }
        }
    }
}

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

@BindingAdapter("reveal")
fun View.setRevealed(reveal: Boolean) {
    val x = measuredWidth
    val y = measuredHeight
    val maxRadius = hypot(x.toDouble(), y.toDouble()).toFloat()
    val start = if (reveal) 0f else maxRadius
    val end = if (reveal) maxRadius else 0f

    val anim = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
        isInvisible = reveal
        return
    } else {
        ViewAnimationUtils.createCircularReveal(this, x, 0, start, end).apply {
            interpolator = FastOutSlowInInterpolator()
            setTag(R.id.revealAnim, this)
            doOnEnd { setTag(R.id.revealAnim, null) }
        }
    }

    post {
        isVisible = true
        anim.start()
    }
}

@BindingAdapter("revealFix")
fun View.setFixReveal(isRevealed: Boolean) {
    (getTag(R.id.revealAnim) as? Animator)
        ?.doOnEnd { isInvisible = !isRevealed }
        ?.let { return }

    isInvisible = !isRevealed
}

@BindingAdapter("dividerVertical", "dividerHorizontal", requireAll = false)
fun RecyclerView.setDividers(dividerVertical: Int, dividerHorizontal: Int) {
    val horizontal = if (dividerHorizontal > 0) {
        context.drawableCompat(dividerHorizontal)
    } else {
        null
    }
    val vertical = if (dividerVertical > 0) {
        context.drawableCompat(dividerVertical)
    } else {
        null
    }
    setDividers(vertical, horizontal)
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

@BindingAdapter("rotationAnimated")
fun View.rotationTo(value: Int) {
    animate()
        .rotation(value.toFloat())
        .setInterpolator(FastOutSlowInInterpolator())
        .start()
}

@BindingAdapter("app:icon")
fun MaterialButton.setIconRes(res: Int) {
    setIconResource(res)
}

@BindingAdapter("cardElevation")
fun MaterialCardView.setCardElevationBound(elevation: Float) {
    cardElevation = elevation
}

@BindingAdapter("strokeWidth")
fun MaterialCardView.setCardStrokeWidthBound(stroke: Float) {
    strokeWidth = stroke.roundToInt()
}

@BindingAdapter("onMenuClick")
fun Toolbar.setOnMenuClickListener(listener: Toolbar.OnMenuItemClickListener) {
    setOnMenuItemClickListener(listener)
}

@BindingAdapter("tooltipText")
fun View.setTooltipTextCompat(text: String) {
    ViewCompat.setTooltipText(this, text)
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

@BindingAdapter("android:rotation")
fun View.setRotationNotAnimated(rotation: Int) {
    if (animation != null) {
        this.rotation = rotation.toFloat()
    }
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
    tag = tag as? PopupMenu ?: let {
        val themeWrapper = ContextThemeWrapper(context, R.style.Foundation_PopupMenu)
        PopupMenu(themeWrapper, this)
    }
    setOnClickListener {
        (tag as PopupMenu).apply {
            menuInflater.inflate(popupMenu, menu)
            setOnMenuItemClickListener {
                listener.onMenuItemClick(it.itemId)
                true
            }
            show()
        }
    }
}