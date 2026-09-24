package com.topjohnwu.magisk.ui.terminal

import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.PorterDuff
import android.graphics.Typeface
import com.topjohnwu.magisk.terminal.TerminalEmulator
import com.topjohnwu.magisk.terminal.TextStyle
import com.topjohnwu.magisk.terminal.WcWidth

/**
 * Renderer of a [TerminalEmulator] into a [Canvas].
 *
 * Saves font metrics, so needs to be recreated each time the typeface or font size changes.
 */
class TerminalRenderer(
    textSize: Int,
    typeface: Typeface,
) {
    val textSize: Int = textSize
    val typeface: Typeface = typeface
    private val textPaint = Paint()

    /** The width of a single mono spaced character obtained by [Paint.measureText] on a single 'X'. */
    val fontWidth: Float

    /** The [Paint.getFontSpacing]. See http://www.fampennings.nl/maarten/android/08numgrid/font.png */
    val fontLineSpacing: Int

    /** The [Paint.ascent]. See http://www.fampennings.nl/maarten/android/08numgrid/font.png */
    private val fontAscent: Int

    /** The [fontLineSpacing] + [fontAscent]. */
    val fontLineSpacingAndAscent: Int

    private val asciiMeasures = FloatArray(127)

    init {
        textPaint.typeface = typeface
        textPaint.isAntiAlias = true
        textPaint.textSize = textSize.toFloat()

        fontLineSpacing = kotlin.math.ceil(textPaint.fontSpacing).toInt()
        fontAscent = kotlin.math.ceil(textPaint.ascent()).toInt()
        fontLineSpacingAndAscent = fontLineSpacing + fontAscent
        fontWidth = textPaint.measureText("X")

        val sb = StringBuilder(" ")
        for (i in asciiMeasures.indices) {
            sb[0] = i.toChar()
            asciiMeasures[i] = textPaint.measureText(sb, 0, 1)
        }
    }

    /**
     * Render the terminal to a canvas with at a specified row scroll, and an optional rectangular selection.
     */
    fun render(
        mEmulator: TerminalEmulator,
        canvas: Canvas,
        topRow: Int,
        selectionY1: Int,
        selectionY2: Int,
        selectionX1: Int,
        selectionX2: Int,
    ) {
        val reverseVideo = mEmulator.isReverseVideo
        val endRow = topRow + mEmulator.mRows
        val columns = mEmulator.mColumns
        val cursorCol = mEmulator.cursorCol
        val cursorRow = mEmulator.cursorRow
        val cursorVisible = mEmulator.shouldCursorBeVisible()
        val screen = mEmulator.screen
        val palette = mEmulator.mColors.currentColors
        val cursorShape = mEmulator.cursorStyle

        if (reverseVideo) {
            canvas.drawColor(palette[TextStyle.COLOR_INDEX_FOREGROUND], PorterDuff.Mode.SRC)
        }

        var heightOffset = fontLineSpacingAndAscent.toFloat()
        for (row in topRow until endRow) {
            heightOffset += fontLineSpacing

            val cursorX = if (row == cursorRow && cursorVisible) cursorCol else -1
            var selx1 = -1
            var selx2 = -1
            if (row in selectionY1..selectionY2) {
                if (row == selectionY1) selx1 = selectionX1
                selx2 = if (row == selectionY2) selectionX2 else mEmulator.mColumns
            }

            val lineObject = screen.allocateFullLineIfNecessary(screen.externalToInternalRow(row))
            val line = lineObject.text
            val charsUsedInLine = lineObject.spaceUsed

            var lastRunStyle = 0L
            var lastRunInsideCursor = false
            var lastRunInsideSelection = false
            var lastRunStartColumn = -1
            var lastRunStartIndex = 0
            var lastRunFontWidthMismatch = false
            var currentCharIndex = 0
            var measuredWidthForRun = 0f

            var column = 0
            while (column < columns) {
                val charAtIndex = line[currentCharIndex]
                val charIsHighsurrogate = Character.isHighSurrogate(charAtIndex)
                val charsForCodePoint = if (charIsHighsurrogate) 2 else 1
                val codePoint = if (charIsHighsurrogate) {
                    Character.toCodePoint(charAtIndex, line[currentCharIndex + 1])
                } else {
                    charAtIndex.code
                }
                val codePointWcWidth = WcWidth.width(codePoint)
                val insideCursor = cursorX == column || (codePointWcWidth == 2 && cursorX == column + 1)
                val insideSelection = column >= selx1 && column <= selx2
                val style = lineObject.getStyle(column)

                // Check if the measured text width for this code point is not the same as that expected by wcwidth().
                // This could happen for some fonts which are not truly monospace, or for more exotic characters such as
                // smileys which android font renders as wide.
                // If this is detected, we draw this code point scaled to match what wcwidth() expects.
                val measuredCodePointWidth = if (codePoint < asciiMeasures.size) {
                    asciiMeasures[codePoint]
                } else {
                    textPaint.measureText(line, currentCharIndex, charsForCodePoint)
                }
                val fontWidthMismatch = kotlin.math.abs(measuredCodePointWidth / fontWidth - codePointWcWidth.toFloat()) > 0.01f

                if (style != lastRunStyle || insideCursor != lastRunInsideCursor || insideSelection != lastRunInsideSelection ||
                    fontWidthMismatch || lastRunFontWidthMismatch
                ) {
                    if (column != 0) {
                        val columnWidthSinceLastRun = column - lastRunStartColumn
                        val charsSinceLastRun = currentCharIndex - lastRunStartIndex
                        val cursorColor = if (lastRunInsideCursor) mEmulator.mColors.currentColors[TextStyle.COLOR_INDEX_CURSOR] else 0
                        var invertCursorTextColor = false
                        if (lastRunInsideCursor && cursorShape == TerminalEmulator.TERMINAL_CURSOR_STYLE_BLOCK) {
                            invertCursorTextColor = true
                        }
                        drawTextRun(
                            canvas, line, palette, heightOffset, lastRunStartColumn, columnWidthSinceLastRun,
                            lastRunStartIndex, charsSinceLastRun, measuredWidthForRun,
                            cursorColor, cursorShape, lastRunStyle,
                            reverseVideo || invertCursorTextColor || lastRunInsideSelection,
                        )
                    }
                    measuredWidthForRun = 0f
                    lastRunStyle = style
                    lastRunInsideCursor = insideCursor
                    lastRunInsideSelection = insideSelection
                    lastRunStartColumn = column
                    lastRunStartIndex = currentCharIndex
                    lastRunFontWidthMismatch = fontWidthMismatch
                }
                measuredWidthForRun += measuredCodePointWidth
                column += codePointWcWidth
                currentCharIndex += charsForCodePoint
                while (currentCharIndex < charsUsedInLine && WcWidth.width(line, currentCharIndex) <= 0) {
                    // Eat combining chars so that they are treated as part of the last non-combining code point,
                    // instead of e.g. being considered inside the cursor in the next run.
                    currentCharIndex += if (Character.isHighSurrogate(line[currentCharIndex])) 2 else 1
                }
            }

            val columnWidthSinceLastRun = columns - lastRunStartColumn
            val charsSinceLastRun = currentCharIndex - lastRunStartIndex
            val cursorColor = if (lastRunInsideCursor) mEmulator.mColors.currentColors[TextStyle.COLOR_INDEX_CURSOR] else 0
            var invertCursorTextColor = false
            if (lastRunInsideCursor && cursorShape == TerminalEmulator.TERMINAL_CURSOR_STYLE_BLOCK) {
                invertCursorTextColor = true
            }
            drawTextRun(
                canvas, line, palette, heightOffset, lastRunStartColumn, columnWidthSinceLastRun,
                lastRunStartIndex, charsSinceLastRun, measuredWidthForRun,
                cursorColor, cursorShape, lastRunStyle,
                reverseVideo || invertCursorTextColor || lastRunInsideSelection,
            )
        }
    }

    private fun drawTextRun(
        canvas: Canvas,
        text: CharArray,
        palette: IntArray,
        y: Float,
        startColumn: Int,
        runWidthColumns: Int,
        startCharIndex: Int,
        runWidthChars: Int,
        mes: Float,
        cursor: Int,
        cursorStyle: Int,
        textStyle: Long,
        reverseVideo: Boolean,
    ) {
        var foreColor = TextStyle.decodeForeColor(textStyle)
        val effect = TextStyle.decodeEffect(textStyle)
        var backColor = TextStyle.decodeBackColor(textStyle)
        val bold = (effect and (TextStyle.CHARACTER_ATTRIBUTE_BOLD or TextStyle.CHARACTER_ATTRIBUTE_BLINK)) != 0
        val underline = (effect and TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE) != 0
        val italic = (effect and TextStyle.CHARACTER_ATTRIBUTE_ITALIC) != 0
        val strikeThrough = (effect and TextStyle.CHARACTER_ATTRIBUTE_STRIKETHROUGH) != 0
        val dim = (effect and TextStyle.CHARACTER_ATTRIBUTE_DIM) != 0

        if ((foreColor and 0xff000000.toInt()) != 0xff000000.toInt()) {
            // Let bold have bright colors if applicable (one of the first 8):
            if (bold && foreColor in 0..7) foreColor += 8
            foreColor = palette[foreColor]
        }

        if ((backColor and 0xff000000.toInt()) != 0xff000000.toInt()) {
            backColor = palette[backColor]
        }

        // Reverse video here if _one and only one_ of the reverse flags are set:
        val reverseVideoHere = reverseVideo xor ((effect and TextStyle.CHARACTER_ATTRIBUTE_INVERSE) != 0)
        if (reverseVideoHere) {
            val tmp = foreColor
            foreColor = backColor
            backColor = tmp
        }

        var left = startColumn * fontWidth
        var right = left + runWidthColumns * fontWidth

        var adjustedMes = mes / fontWidth
        var savedMatrix = false
        if (kotlin.math.abs(adjustedMes - runWidthColumns) > 0.01) {
            canvas.save()
            canvas.scale(runWidthColumns / adjustedMes, 1f)
            left *= adjustedMes / runWidthColumns
            right *= adjustedMes / runWidthColumns
            savedMatrix = true
        }

        if (backColor != palette[TextStyle.COLOR_INDEX_BACKGROUND]) {
            // Only draw non-default background.
            textPaint.color = backColor
            canvas.drawRect(left, y - fontLineSpacingAndAscent + fontAscent, right, y, textPaint)
        }

        if (cursor != 0) {
            textPaint.color = cursor
            var cursorHeight = (fontLineSpacingAndAscent - fontAscent).toFloat()
            if (cursorStyle == TerminalEmulator.TERMINAL_CURSOR_STYLE_UNDERLINE) cursorHeight /= 4f
            else if (cursorStyle == TerminalEmulator.TERMINAL_CURSOR_STYLE_BAR) right -= ((right - left) * 3) / 4f
            canvas.drawRect(left, y - cursorHeight, right, y, textPaint)
        }

        if ((effect and TextStyle.CHARACTER_ATTRIBUTE_INVISIBLE) == 0) {
            if (dim) {
                var red = 0xFF and (foreColor shr 16)
                var green = 0xFF and (foreColor shr 8)
                var blue = 0xFF and foreColor
                // Dim color handling used by libvte which in turn took it from xterm
                // (https://bug735245.bugzilla-attachments.gnome.org/attachment.cgi?id=284267):
                red = red * 2 / 3
                green = green * 2 / 3
                blue = blue * 2 / 3
                foreColor = -0x1000000 or (red shl 16) or (green shl 8) or blue
            }

            textPaint.isFakeBoldText = bold
            textPaint.isUnderlineText = underline
            textPaint.textSkewX = if (italic) -0.35f else 0f
            textPaint.isStrikeThruText = strikeThrough
            textPaint.color = foreColor

            // The text alignment is the default Paint.Align.LEFT.
            canvas.drawTextRun(
                text, startCharIndex, runWidthChars, startCharIndex, runWidthChars,
                left, y - fontLineSpacingAndAscent, false, textPaint,
            )
        }

        if (savedMatrix) canvas.restore()
    }
}
