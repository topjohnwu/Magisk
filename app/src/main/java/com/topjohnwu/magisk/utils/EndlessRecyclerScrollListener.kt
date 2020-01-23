package com.topjohnwu.magisk.utils

import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.model.events.ViewEvent

class EndlessRecyclerScrollListener(
    private val layoutManager: RecyclerView.LayoutManager,
    private val loadMore: (page: Int, totalItemsCount: Int, view: RecyclerView?) -> Unit,
    private val direction: Direction = Direction.BOTTOM,
    visibleRowsThreshold: Int = VISIBLE_THRESHOLD
) : RecyclerView.OnScrollListener() {

    constructor(
        layoutManager: RecyclerView.LayoutManager,
        loadMore: () -> Unit,
        direction: Direction = Direction.BOTTOM,
        visibleRowsThreshold: Int = VISIBLE_THRESHOLD
    ) : this(layoutManager, { _, _, _ -> loadMore() }, direction, visibleRowsThreshold)

    enum class Direction {
        TOP, BOTTOM
    }

    companion object {
        private const val VISIBLE_THRESHOLD = 5
        private const val STARTING_PAGE_INDEX = 0
    }

    // The minimum amount of items to have above/below your current scroll position
    // before loading more.
    private val visibleThreshold = when (layoutManager) {
        is LinearLayoutManager -> visibleRowsThreshold
        is GridLayoutManager -> visibleRowsThreshold * layoutManager.spanCount
        is StaggeredGridLayoutManager -> visibleRowsThreshold * layoutManager.spanCount
        else -> throw IllegalArgumentException("Only LinearLayoutManager, GridLayoutManager and StaggeredGridLayoutManager are supported")
    }

    // The current offset index of data you have loaded
    private var currentPage = 0
    // The total number of items in the dataset after the last load
    private var previousTotalItemCount = 0
    // True if we are still waiting for the last set of data to load.
    private var loading = true

    // This happens many times a second during a scroll, so be wary of the code you place here.
    // We are given a few useful parameters to help us work out if we need to load some more data,
    // but first we check if we are waiting for the previous load to finish.
    override fun onScrolled(view: RecyclerView, dx: Int, dy: Int) {
        if (dx == 0 && dy == 0) return
        val totalItemCount = layoutManager.itemCount

        val visibleItemPosition = if (direction == Direction.BOTTOM) {
            when (layoutManager) {
                is StaggeredGridLayoutManager -> layoutManager.findLastVisibleItemPositions(null).max()
                    ?: 0
                is GridLayoutManager -> layoutManager.findLastVisibleItemPosition()
                is LinearLayoutManager -> layoutManager.findLastVisibleItemPosition()
                else -> throw IllegalArgumentException("Only LinearLayoutManager, GridLayoutManager and StaggeredGridLayoutManager are supported")
            }
        } else {
            when (layoutManager) {
                is StaggeredGridLayoutManager -> layoutManager.findFirstVisibleItemPositions(null).min()
                    ?: 0
                is GridLayoutManager -> layoutManager.findFirstVisibleItemPosition()
                is LinearLayoutManager -> layoutManager.findFirstVisibleItemPosition()
                else -> throw IllegalArgumentException("Only LinearLayoutManager, GridLayoutManager and StaggeredGridLayoutManager are supported")
            }
        }

        // If the total item count is zero and the previous isn't, assume the
        // list is invalidated and should be reset back to initial state
        if (totalItemCount < previousTotalItemCount) {
            currentPage =
                STARTING_PAGE_INDEX
            previousTotalItemCount = totalItemCount
            if (totalItemCount == 0) {
                loading = true
            }
        }

        // If it’s still loading, we check to see if the dataset count has
        // changed, if so we conclude it has finished loading and update the current page
        // number and total item count.
        if (loading && totalItemCount > previousTotalItemCount) {
            loading = false
            previousTotalItemCount = totalItemCount
        }

        // If it isn’t currently loading, we check to see if we have breached
        // the visibleThreshold and need to reload more data.
        // If we do need to reload some more data, we execute onLoadMore to fetch the data.
        // threshold should reflect how many total columns there are too
        if (!loading && shouldLoadMoreItems(visibleItemPosition, totalItemCount)) {
            currentPage++
            loadMore(currentPage, totalItemCount, view)
            loading = true
        }
    }

    private fun shouldLoadMoreItems(visibleItemPosition: Int, itemCount: Int) = when (direction) {
        Direction.TOP -> visibleItemPosition < visibleThreshold
        Direction.BOTTOM -> visibleItemPosition + visibleThreshold > itemCount
    }

    // Call this method whenever performing new searches
    fun resetState() {
        currentPage = STARTING_PAGE_INDEX
        previousTotalItemCount = 0
        loading = true
    }

    class ResetState : ViewEvent()
}