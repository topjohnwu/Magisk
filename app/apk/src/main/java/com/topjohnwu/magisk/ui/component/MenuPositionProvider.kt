package com.topjohnwu.magisk.ui.component

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.IntRect
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.LayoutDirection
import androidx.compose.ui.unit.dp
import top.yukonga.miuix.kmp.basic.PopupPositionProvider

object ListPopupDefaults {
    val MenuPositionProvider = object : PopupPositionProvider {
        override fun calculatePosition(
            anchorBounds: IntRect,
            windowBounds: IntRect,
            layoutDirection: LayoutDirection,
            popupContentSize: IntSize,
            popupMargin: IntRect,
            alignment: PopupPositionProvider.Align,
        ): IntOffset {
            val resolved = alignment.resolve(layoutDirection)
            val offsetX: Int
            val offsetY: Int
            when (resolved) {
                PopupPositionProvider.Align.TopStart -> {
                    offsetX = anchorBounds.left + popupMargin.left
                    offsetY = anchorBounds.bottom + popupMargin.top
                }
                PopupPositionProvider.Align.TopEnd -> {
                    offsetX = anchorBounds.right - popupContentSize.width - popupMargin.right
                    offsetY = anchorBounds.bottom + popupMargin.top
                }
                PopupPositionProvider.Align.BottomStart -> {
                    offsetX = anchorBounds.left + popupMargin.left
                    offsetY = anchorBounds.top - popupContentSize.height - popupMargin.bottom
                }
                PopupPositionProvider.Align.BottomEnd -> {
                    offsetX = anchorBounds.right - popupContentSize.width - popupMargin.right
                    offsetY = anchorBounds.top - popupContentSize.height - popupMargin.bottom
                }
                else -> {
                    offsetX = if (resolved == PopupPositionProvider.Align.End) {
                        anchorBounds.right - popupContentSize.width - popupMargin.right
                    } else {
                        anchorBounds.left + popupMargin.left
                    }
                    offsetY = if (windowBounds.bottom - anchorBounds.bottom > popupContentSize.height) {
                        anchorBounds.bottom + popupMargin.bottom
                    } else if (anchorBounds.top - windowBounds.top > popupContentSize.height) {
                        anchorBounds.top - popupContentSize.height - popupMargin.top
                    } else {
                        anchorBounds.top + anchorBounds.height / 2 - popupContentSize.height / 2
                    }
                }
            }
            return IntOffset(
                x = offsetX.coerceIn(
                    windowBounds.left,
                    (windowBounds.right - popupContentSize.width - popupMargin.right)
                        .coerceAtLeast(windowBounds.left),
                ),
                y = offsetY.coerceIn(
                    (windowBounds.top + popupMargin.top)
                        .coerceAtMost(windowBounds.bottom - popupContentSize.height - popupMargin.bottom),
                    windowBounds.bottom - popupContentSize.height - popupMargin.bottom,
                ),
            )
        }

        override fun getMargins(): PaddingValues = PaddingValues(start = 20.dp)
    }
}

private fun PopupPositionProvider.Align.resolve(layoutDirection: LayoutDirection): PopupPositionProvider.Align {
    if (layoutDirection == LayoutDirection.Ltr) return this
    return when (this) {
        PopupPositionProvider.Align.Start -> PopupPositionProvider.Align.End
        PopupPositionProvider.Align.End -> PopupPositionProvider.Align.Start
        PopupPositionProvider.Align.TopStart -> PopupPositionProvider.Align.TopEnd
        PopupPositionProvider.Align.TopEnd -> PopupPositionProvider.Align.TopStart
        PopupPositionProvider.Align.BottomStart -> PopupPositionProvider.Align.BottomEnd
        PopupPositionProvider.Align.BottomEnd -> PopupPositionProvider.Align.BottomStart
    }
}
