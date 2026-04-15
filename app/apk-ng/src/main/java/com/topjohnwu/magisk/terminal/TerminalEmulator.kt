package com.topjohnwu.magisk.terminal

import android.util.Base64
import timber.log.Timber
import java.util.Stack

/**
 * Renders text into a screen. Contains all the terminal-specific knowledge and state. Emulates a subset of the X Window
 * System xterm terminal, which in turn is an emulator for a subset of the Digital Equipment Corporation vt100 terminal.
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://en.wikipedia.org/wiki/ANSI_escape_code
 * - http://man.he.net/man4/console_codes
 * - http://bazaar.launchpad.net/~leonerd/libvterm/trunk/view/head:/src/state.c
 * - http://www.columbia.edu/~kermit/k95manual/iso2022.html
 * - http://www.vt100.net/docs/vt510-rm/chapter4
 * - http://en.wikipedia.org/wiki/ISO/IEC_2022 - for 7-bit and 8-bit GL GR explanation
 * - http://bjh21.me.uk/all-escapes/all-escapes.txt - extensive!
 * - http://woldlab.caltech.edu/~diane/kde4.10/workingdir/kubuntu/konsole/doc/developer/old-documents/VT100/techref.html
 */
class TerminalEmulator(
    columns: Int,
    rows: Int,
    cellWidthPixels: Int,
    cellHeightPixels: Int,
    transcriptRows: Int?,
) {

    companion object {
        private const val LOG_ESCAPE_SEQUENCES = false

        /** Used for invalid data - http://en.wikipedia.org/wiki/Replacement_character#Replacement_character */
        const val UNICODE_REPLACEMENT_CHAR = 0xFFFD

        private const val ESC_NONE = 0
        private const val ESC = 1
        private const val ESC_POUND = 2
        private const val ESC_SELECT_LEFT_PAREN = 3
        private const val ESC_SELECT_RIGHT_PAREN = 4
        private const val ESC_CSI = 6
        private const val ESC_CSI_QUESTIONMARK = 7
        private const val ESC_CSI_DOLLAR = 8
        private const val ESC_PERCENT = 9
        private const val ESC_OSC = 10
        private const val ESC_OSC_ESC = 11
        private const val ESC_CSI_BIGGERTHAN = 12
        private const val ESC_P = 13
        private const val ESC_CSI_QUESTIONMARK_ARG_DOLLAR = 14
        private const val ESC_CSI_ARGS_SPACE = 15
        private const val ESC_CSI_ARGS_ASTERIX = 16
        private const val ESC_CSI_DOUBLE_QUOTE = 17
        private const val ESC_CSI_SINGLE_QUOTE = 18
        private const val ESC_CSI_EXCLAMATION = 19
        private const val ESC_APC = 20
        private const val ESC_APC_ESCAPE = 21
        private const val ESC_CSI_UNSUPPORTED_PARAMETER_BYTE = 22
        private const val ESC_CSI_UNSUPPORTED_INTERMEDIATE_BYTE = 23

        private const val MAX_ESCAPE_PARAMETERS = 32
        private const val MAX_OSC_STRING_LENGTH = 8192

        private const val DECSET_BIT_APPLICATION_CURSOR_KEYS = 1
        private const val DECSET_BIT_REVERSE_VIDEO = 1 shl 1
        private const val DECSET_BIT_ORIGIN_MODE = 1 shl 2
        private const val DECSET_BIT_AUTOWRAP = 1 shl 3
        private const val DECSET_BIT_CURSOR_ENABLED = 1 shl 4
        private const val DECSET_BIT_APPLICATION_KEYPAD = 1 shl 5
        private const val DECSET_BIT_MOUSE_TRACKING_PRESS_RELEASE = 1 shl 6
        private const val DECSET_BIT_MOUSE_TRACKING_BUTTON_EVENT = 1 shl 7
        private const val DECSET_BIT_SEND_FOCUS_EVENTS = 1 shl 8
        private const val DECSET_BIT_MOUSE_PROTOCOL_SGR = 1 shl 9
        private const val DECSET_BIT_BRACKETED_PASTE_MODE = 1 shl 10
        private const val DECSET_BIT_LEFTRIGHT_MARGIN_MODE = 1 shl 11
        private const val DECSET_BIT_RECTANGULAR_CHANGEATTRIBUTE = 1 shl 12

        const val TERMINAL_TRANSCRIPT_ROWS_MIN = 100
        const val TERMINAL_TRANSCRIPT_ROWS_MAX = 50000
        const val DEFAULT_TERMINAL_TRANSCRIPT_ROWS = 2000

        const val TERMINAL_CURSOR_STYLE_BLOCK = 0
        const val TERMINAL_CURSOR_STYLE_UNDERLINE = 1
        const val TERMINAL_CURSOR_STYLE_BAR = 2
        const val DEFAULT_TERMINAL_CURSOR_STYLE = TERMINAL_CURSOR_STYLE_BLOCK

        val TERMINAL_CURSOR_STYLES_LIST = intArrayOf(
            TERMINAL_CURSOR_STYLE_BLOCK,
            TERMINAL_CURSOR_STYLE_UNDERLINE,
            TERMINAL_CURSOR_STYLE_BAR
        )

        private const val LOG_TAG = "TerminalEmulator"

        fun mapDecSetBitToInternalBit(decsetBit: Int): Int = when (decsetBit) {
            1 -> DECSET_BIT_APPLICATION_CURSOR_KEYS
            5 -> DECSET_BIT_REVERSE_VIDEO
            6 -> DECSET_BIT_ORIGIN_MODE
            7 -> DECSET_BIT_AUTOWRAP
            25 -> DECSET_BIT_CURSOR_ENABLED
            66 -> DECSET_BIT_APPLICATION_KEYPAD
            69 -> DECSET_BIT_LEFTRIGHT_MARGIN_MODE
            1000 -> DECSET_BIT_MOUSE_TRACKING_PRESS_RELEASE
            1002 -> DECSET_BIT_MOUSE_TRACKING_BUTTON_EVENT
            1004 -> DECSET_BIT_SEND_FOCUS_EVENTS
            1006 -> DECSET_BIT_MOUSE_PROTOCOL_SGR
            2004 -> DECSET_BIT_BRACKETED_PASTE_MODE
            else -> -1
        }
    }

    var title: String? = null
        private set
    private val titleStack = Stack<String>()

    var cursorRow = 0
        private set
    var cursorCol = 0
        private set

    var mRows: Int = rows
    var mColumns: Int = columns

    private var mCellWidthPixels: Int = cellWidthPixels
    private var mCellHeightPixels: Int = cellHeightPixels

    var cursorStyle = DEFAULT_TERMINAL_CURSOR_STYLE
        private set

    private val mMainBuffer: TerminalBuffer
    internal val mAltBuffer: TerminalBuffer
    var screen: TerminalBuffer
        private set

    var onCopyToClipboard: ((String) -> Unit)? = null
    var onScreenUpdate: (() -> Unit)? = null

    private var mArgIndex = 0
    private val mArgs = IntArray(MAX_ESCAPE_PARAMETERS)
    private var mArgsSubParamsBitSet = 0

    private val mOSCOrDeviceControlArgs = StringBuilder()

    private var mContinueSequence = false
    private var mEscapeState = ESC_NONE

    private val mSavedStateMain = SavedScreenState()
    private val mSavedStateAlt = SavedScreenState()

    private var mUseLineDrawingG0 = false
    private var mUseLineDrawingG1 = false
    private var mUseLineDrawingUsesG0 = true

    private var mCurrentDecSetFlags = 0
    private var mSavedDecSetFlags = 0

    private var mInsertMode = false
    private var mTabStop: BooleanArray

    private var mTopMargin = 0
    private var mBottomMargin = 0
    private var mLeftMargin = 0
    private var mRightMargin = 0

    private var mAboutToAutoWrap = false
    private var mCursorBlinkingEnabled = false
    private var mCursorBlinkState = false

    private var mForeColor = 0
    private var mBackColor = 0
    private var mUnderlineColor = 0
    private var mEffect = 0

    var scrollCounter = 0
        private set
    var isAutoScrollDisabled = false
        private set

    private var mUtf8ToFollow = 0
    private var mUtf8Index = 0
    private val mUtf8InputBuffer = ByteArray(4)
    private var mLastEmittedCodePoint = -1

    val mColors = TerminalColors()

    init {
        mMainBuffer = TerminalBuffer(columns, getTerminalTranscriptRows(transcriptRows), rows)
        mAltBuffer = TerminalBuffer(columns, rows, rows)
        screen = mMainBuffer
        mTabStop = BooleanArray(mColumns)
        reset()
    }

    val isAlternateBufferActive: Boolean get() = screen === mAltBuffer

    private fun getTerminalTranscriptRows(transcriptRows: Int?): Int {
        return if (transcriptRows == null || transcriptRows < TERMINAL_TRANSCRIPT_ROWS_MIN || transcriptRows > TERMINAL_TRANSCRIPT_ROWS_MAX)
            DEFAULT_TERMINAL_TRANSCRIPT_ROWS
        else
            transcriptRows
    }

    fun resize(columns: Int, rows: Int, cellWidthPixels: Int, cellHeightPixels: Int) {
        this.mCellWidthPixels = cellWidthPixels
        this.mCellHeightPixels = cellHeightPixels

        if (mRows == rows && mColumns == columns) {
            return
        } else if (columns < 2 || rows < 2) {
            throw IllegalArgumentException("rows=$rows, columns=$columns")
        }

        if (mRows != rows) {
            mRows = rows
            mTopMargin = 0
            mBottomMargin = mRows
        }
        if (mColumns != columns) {
            val oldColumns = mColumns
            mColumns = columns
            val oldTabStop = mTabStop
            mTabStop = BooleanArray(mColumns)
            setDefaultTabStops()
            val toTransfer = minOf(oldColumns, columns)
            System.arraycopy(oldTabStop, 0, mTabStop, 0, toTransfer)
            mLeftMargin = 0
            mRightMargin = mColumns
        }

        resizeScreen()
    }

    private fun resizeScreen() {
        val cursor = intArrayOf(cursorCol, cursorRow)
        val newTotalRows = if (screen === mAltBuffer) mRows else mMainBuffer.totalRows
        screen.resize(mColumns, mRows, newTotalRows, cursor, style, isAlternateBufferActive)
        cursorCol = cursor[0]
        cursorRow = cursor[1]
    }

    fun setCursorStyle() {
        cursorStyle = DEFAULT_TERMINAL_CURSOR_STYLE
    }

    val isReverseVideo: Boolean get() = isDecsetInternalBitSet(DECSET_BIT_REVERSE_VIDEO)

    val isCursorEnabled: Boolean get() = isDecsetInternalBitSet(DECSET_BIT_CURSOR_ENABLED)

    fun shouldCursorBeVisible(): Boolean {
        if (!isCursorEnabled) return false
        return if (mCursorBlinkingEnabled) mCursorBlinkState else true
    }

    fun setCursorBlinkingEnabled(cursorBlinkingEnabled: Boolean) {
        this.mCursorBlinkingEnabled = cursorBlinkingEnabled
    }

    fun setCursorBlinkState(cursorBlinkState: Boolean) {
        this.mCursorBlinkState = cursorBlinkState
    }

    fun isKeypadApplicationMode(): Boolean = isDecsetInternalBitSet(DECSET_BIT_APPLICATION_KEYPAD)

    private fun isDecsetInternalBitSet(bit: Int): Boolean = (mCurrentDecSetFlags and bit) != 0

    private fun setDecsetinternalBit(internalBit: Int, set: Boolean) {
        if (set) {
            if (internalBit == DECSET_BIT_MOUSE_TRACKING_PRESS_RELEASE) {
                setDecsetinternalBit(DECSET_BIT_MOUSE_TRACKING_BUTTON_EVENT, false)
            } else if (internalBit == DECSET_BIT_MOUSE_TRACKING_BUTTON_EVENT) {
                setDecsetinternalBit(DECSET_BIT_MOUSE_TRACKING_PRESS_RELEASE, false)
            }
        }
        if (set) {
            mCurrentDecSetFlags = mCurrentDecSetFlags or internalBit
        } else {
            mCurrentDecSetFlags = mCurrentDecSetFlags and internalBit.inv()
        }
    }

    private fun setDefaultTabStops() {
        for (i in 0 until mColumns)
            mTabStop[i] = (i and 7) == 0 && i != 0
    }

    fun append(buffer: ByteArray, length: Int) {
        for (i in 0 until length)
            processByte(buffer[i])
    }

    private fun processByte(byteToProcess: Byte) {
        if (mUtf8ToFollow > 0) {
            if ((byteToProcess.toInt() and 0b11000000) == 0b10000000) {
                mUtf8InputBuffer[mUtf8Index] = byteToProcess
                mUtf8Index++
                if (--mUtf8ToFollow == 0) {
                    val firstByteMask = when (mUtf8Index) {
                        2 -> 0b00011111
                        3 -> 0b00001111
                        else -> 0b00000111
                    }
                    var codePoint = mUtf8InputBuffer[0].toInt() and firstByteMask
                    for (i in 1 until mUtf8Index)
                        codePoint = (codePoint shl 6) or (mUtf8InputBuffer[i].toInt() and 0b00111111)
                    if (((codePoint <= 0b1111111) && mUtf8Index > 1) || (codePoint < 0b11111111111 && mUtf8Index > 2)
                        || (codePoint < 0b1111111111111111 && mUtf8Index > 3)
                    ) {
                        codePoint = UNICODE_REPLACEMENT_CHAR
                    }

                    mUtf8Index = 0
                    mUtf8ToFollow = 0

                    if (codePoint in 0x80..0x9F) {
                        // C1 control character, ignore
                    } else {
                        when (Character.getType(codePoint).toByte()) {
                            Character.UNASSIGNED, Character.SURROGATE ->
                                codePoint = UNICODE_REPLACEMENT_CHAR
                        }
                        processCodePoint(codePoint)
                    }
                }
            } else {
                mUtf8Index = 0
                mUtf8ToFollow = 0
                emitCodePoint(UNICODE_REPLACEMENT_CHAR)
                processByte(byteToProcess)
            }
        } else {
            if ((byteToProcess.toInt() and 0b10000000) == 0) {
                processCodePoint(byteToProcess.toInt())
                return
            } else if ((byteToProcess.toInt() and 0b11100000) == 0b11000000) {
                mUtf8ToFollow = 1
            } else if ((byteToProcess.toInt() and 0b11110000) == 0b11100000) {
                mUtf8ToFollow = 2
            } else if ((byteToProcess.toInt() and 0b11111000) == 0b11110000) {
                mUtf8ToFollow = 3
            } else {
                processCodePoint(UNICODE_REPLACEMENT_CHAR)
                return
            }
            mUtf8InputBuffer[mUtf8Index] = byteToProcess
            mUtf8Index++
        }
    }

    fun processCodePoint(b: Int) {
        if (mEscapeState == ESC_APC) {
            doApc(b)
            return
        } else if (mEscapeState == ESC_APC_ESCAPE) {
            doApcEscape(b)
            return
        }

        when (b) {
            0 -> { /* NUL, do nothing */ }
            7 -> {
                if (mEscapeState == ESC_OSC) doOsc(b)
            }
            8 -> {
                if (mLeftMargin == cursorCol) {
                    val previousRow = cursorRow - 1
                    if (previousRow >= 0 && screen.getLineWrap(previousRow)) {
                        screen.clearLineWrap(previousRow)
                        setCursorRowCol(previousRow, mRightMargin - 1)
                    }
                } else {
                    setCursorCol(cursorCol - 1)
                }
            }
            9 -> cursorCol = nextTabStop(1)
            10, 11, 12 -> doLinefeed()
            13 -> setCursorCol(mLeftMargin)
            14 -> mUseLineDrawingUsesG0 = false
            15 -> mUseLineDrawingUsesG0 = true
            24, 26 -> {
                if (mEscapeState != ESC_NONE) {
                    mEscapeState = ESC_NONE
                    emitCodePoint(127)
                }
            }
            27 -> {
                if (mEscapeState == ESC_P) {
                    return
                } else if (mEscapeState != ESC_OSC) {
                    startEscapeSequence()
                } else {
                    doOsc(b)
                }
            }
            else -> {
                mContinueSequence = false
                when (mEscapeState) {
                    ESC_NONE -> if (b >= 32) emitCodePoint(b)
                    ESC -> doEsc(b)
                    ESC_POUND -> doEscPound(b)
                    ESC_SELECT_LEFT_PAREN -> mUseLineDrawingG0 = (b == '0'.code)
                    ESC_SELECT_RIGHT_PAREN -> mUseLineDrawingG1 = (b == '0'.code)
                    ESC_CSI -> doCsi(b)
                    ESC_CSI_UNSUPPORTED_PARAMETER_BYTE, ESC_CSI_UNSUPPORTED_INTERMEDIATE_BYTE ->
                        doCsiUnsupportedParameterOrIntermediateByte(b)
                    ESC_CSI_EXCLAMATION -> {
                        if (b == 'p'.code) reset() else unknownSequence(b)
                    }
                    ESC_CSI_QUESTIONMARK -> doCsiQuestionMark(b)
                    ESC_CSI_BIGGERTHAN -> doCsiBiggerThan(b)
                    ESC_CSI_DOLLAR -> {
                        val originMode = isDecsetInternalBitSet(DECSET_BIT_ORIGIN_MODE)
                        val effectiveTopMargin = if (originMode) mTopMargin else 0
                        val effectiveBottomMargin = if (originMode) mBottomMargin else mRows
                        val effectiveLeftMargin = if (originMode) mLeftMargin else 0
                        val effectiveRightMargin = if (originMode) mRightMargin else mColumns
                        when (b) {
                            'v'.code -> {
                                val topSource = minOf(getArg(0, 1, true) - 1 + effectiveTopMargin, mRows)
                                val leftSource = minOf(getArg(1, 1, true) - 1 + effectiveLeftMargin, mColumns)
                                val bottomSource = minOf(maxOf(getArg(2, mRows, true) + effectiveTopMargin, topSource), mRows)
                                val rightSource = minOf(maxOf(getArg(3, mColumns, true) + effectiveLeftMargin, leftSource), mColumns)
                                val destionationTop = minOf(getArg(5, 1, true) - 1 + effectiveTopMargin, mRows)
                                val destinationLeft = minOf(getArg(6, 1, true) - 1 + effectiveLeftMargin, mColumns)
                                val heightToCopy = minOf(mRows - destionationTop, bottomSource - topSource)
                                val widthToCopy = minOf(mColumns - destinationLeft, rightSource - leftSource)
                                screen.blockCopy(leftSource, topSource, widthToCopy, heightToCopy, destinationLeft, destionationTop)
                            }
                            '{'.code, 'x'.code, 'z'.code -> {
                                val erase = b != 'x'.code
                                val selective = b == '{'.code
                                val keepVisualAttributes = erase && selective
                                var argIndex = 0
                                val fillChar = if (erase) ' '.code else getArg(argIndex++, -1, true)
                                if ((fillChar in 32..126) || (fillChar in 160..255)) {
                                    val top = minOf(getArg(argIndex++, 1, true) + effectiveTopMargin, effectiveBottomMargin + 1)
                                    val left = minOf(getArg(argIndex++, 1, true) + effectiveLeftMargin, effectiveRightMargin + 1)
                                    val bottom = minOf(getArg(argIndex++, mRows, true) + effectiveTopMargin, effectiveBottomMargin)
                                    val right = minOf(getArg(argIndex, mColumns, true) + effectiveLeftMargin, effectiveRightMargin)
                                    val style = style
                                    for (row in top - 1 until bottom)
                                        for (col in left - 1 until right)
                                            if (!selective || (TextStyle.decodeEffect(screen.getStyleAt(row, col)) and TextStyle.CHARACTER_ATTRIBUTE_PROTECTED) == 0)
                                                screen.setChar(col, row, fillChar, if (keepVisualAttributes) screen.getStyleAt(row, col) else style)
                                }
                            }
                            'r'.code, 't'.code -> {
                                val reverse = b == 't'.code
                                val top = minOf(getArg(0, 1, true) - 1, effectiveBottomMargin) + effectiveTopMargin
                                val left = minOf(getArg(1, 1, true) - 1, effectiveRightMargin) + effectiveLeftMargin
                                val bottom = minOf(getArg(2, mRows, true) + 1, effectiveBottomMargin - 1) + effectiveTopMargin
                                val right = minOf(getArg(3, mColumns, true) + 1, effectiveRightMargin - 1) + effectiveLeftMargin
                                if (mArgIndex >= 4) {
                                    if (mArgIndex >= mArgs.size) mArgIndex = mArgs.size - 1
                                    for (i in 4..mArgIndex) {
                                        var bits = 0
                                        var setOrClear = true
                                        when (getArg(i, 0, false)) {
                                            0 -> {
                                                bits = (TextStyle.CHARACTER_ATTRIBUTE_BOLD or TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE or TextStyle.CHARACTER_ATTRIBUTE_BLINK
                                                    or TextStyle.CHARACTER_ATTRIBUTE_INVERSE)
                                                if (!reverse) setOrClear = false
                                            }
                                            1 -> bits = TextStyle.CHARACTER_ATTRIBUTE_BOLD
                                            4 -> bits = TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE
                                            5 -> bits = TextStyle.CHARACTER_ATTRIBUTE_BLINK
                                            7 -> bits = TextStyle.CHARACTER_ATTRIBUTE_INVERSE
                                            22 -> {
                                                bits = TextStyle.CHARACTER_ATTRIBUTE_BOLD
                                                setOrClear = false
                                            }
                                            24 -> {
                                                bits = TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE
                                                setOrClear = false
                                            }
                                            25 -> {
                                                bits = TextStyle.CHARACTER_ATTRIBUTE_BLINK
                                                setOrClear = false
                                            }
                                            27 -> {
                                                bits = TextStyle.CHARACTER_ATTRIBUTE_INVERSE
                                                setOrClear = false
                                            }
                                        }
                                        if (reverse && !setOrClear) {
                                            // Reverse attributes in rectangular area ignores non-(1,4,5,7) bits.
                                        } else {
                                            screen.setOrClearEffect(
                                                bits, setOrClear, reverse, isDecsetInternalBitSet(DECSET_BIT_RECTANGULAR_CHANGEATTRIBUTE),
                                                effectiveLeftMargin, effectiveRightMargin, top, left, bottom, right
                                            )
                                        }
                                    }
                                }
                            }
                            else -> unknownSequence(b)
                        }
                    }
                    ESC_CSI_DOUBLE_QUOTE -> {
                        if (b == 'q'.code) {
                            val arg = getArg0(0)
                            if (arg == 0 || arg == 2) {
                                mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_PROTECTED.inv()
                            } else if (arg == 1) {
                                mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_PROTECTED
                            } else {
                                unknownSequence(b)
                            }
                        } else {
                            unknownSequence(b)
                        }
                    }
                    ESC_CSI_SINGLE_QUOTE -> {
                        when (b) {
                            '}'.code -> {
                                val columnsAfterCursor = mRightMargin - cursorCol
                                val columnsToInsert = minOf(getArg0(1), columnsAfterCursor)
                                val columnsToMove = columnsAfterCursor - columnsToInsert
                                screen.blockCopy(cursorCol, 0, columnsToMove, mRows, cursorCol + columnsToInsert, 0)
                                blockClear(cursorCol, 0, columnsToInsert, mRows)
                            }
                            '~'.code -> {
                                val columnsAfterCursor = mRightMargin - cursorCol
                                val columnsToDelete = minOf(getArg0(1), columnsAfterCursor)
                                val columnsToMove = columnsAfterCursor - columnsToDelete
                                screen.blockCopy(cursorCol + columnsToDelete, 0, columnsToMove, mRows, cursorCol, 0)
                            }
                            else -> unknownSequence(b)
                        }
                    }
                    ESC_PERCENT -> { /* ignore */ }
                    ESC_OSC -> doOsc(b)
                    ESC_OSC_ESC -> doOscEsc(b)
                    ESC_P -> doDeviceControl(b)
                    ESC_CSI_QUESTIONMARK_ARG_DOLLAR -> {
                        if (b != 'p'.code) unknownSequence(b)
                    }
                    ESC_CSI_ARGS_SPACE -> {
                        val arg = getArg0(0)
                        when (b) {
                            'q'.code -> when (arg) {
                                0, 1, 2 -> cursorStyle = TERMINAL_CURSOR_STYLE_BLOCK
                                3, 4 -> cursorStyle = TERMINAL_CURSOR_STYLE_UNDERLINE
                                5, 6 -> cursorStyle = TERMINAL_CURSOR_STYLE_BAR
                            }
                            't'.code, 'u'.code -> { /* Set margin-bell volume - ignore */ }
                            else -> unknownSequence(b)
                        }
                    }
                    ESC_CSI_ARGS_ASTERIX -> {
                        val attributeChangeExtent = getArg0(0)
                        if (b == 'x'.code && attributeChangeExtent in 0..2) {
                            setDecsetinternalBit(DECSET_BIT_RECTANGULAR_CHANGEATTRIBUTE, attributeChangeExtent == 2)
                        } else {
                            unknownSequence(b)
                        }
                    }
                    else -> unknownSequence(b)
                }
                if (!mContinueSequence) mEscapeState = ESC_NONE
            }
        }
    }

    private fun doDeviceControl(b: Int) {
        when (b) {
            '\\'.code -> {
                val dcs = mOSCOrDeviceControlArgs.toString()
                if (!dcs.startsWith("\$q") && !dcs.startsWith("+q")) {
                    if (LOG_ESCAPE_SEQUENCES)
                        Timber.tag(LOG_TAG).e("Unrecognized device control string: $dcs")
                }
                finishSequence()
            }
            else -> {
                if (mOSCOrDeviceControlArgs.length > MAX_OSC_STRING_LENGTH) {
                    mOSCOrDeviceControlArgs.clear()
                    finishSequence()
                } else {
                    mOSCOrDeviceControlArgs.appendCodePoint(b)
                    continueSequence(mEscapeState)
                }
            }
        }
    }

    private fun doApc(b: Int) {
        if (b == 27) {
            continueSequence(ESC_APC_ESCAPE)
        }
    }

    private fun doApcEscape(b: Int) {
        if (b == '\\'.code) {
            finishSequence()
        } else {
            continueSequence(ESC_APC)
        }
    }

    private fun nextTabStop(numTabs: Int): Int {
        var remaining = numTabs
        for (i in cursorCol + 1 until mColumns)
            if (mTabStop[i] && --remaining == 0) return minOf(i, mRightMargin)
        return mRightMargin - 1
    }

    private fun doCsiUnsupportedParameterOrIntermediateByte(b: Int) {
        if (mEscapeState == ESC_CSI_UNSUPPORTED_PARAMETER_BYTE && b in 0x30..0x3F) {
            continueSequence(ESC_CSI_UNSUPPORTED_PARAMETER_BYTE)
        } else if (b in 0x20..0x2F) {
            continueSequence(ESC_CSI_UNSUPPORTED_INTERMEDIATE_BYTE)
        } else if (b in 0x40..0x7E) {
            finishSequence()
        } else {
            unknownSequence(b)
        }
    }

    private fun doCsiQuestionMark(b: Int) {
        when (b) {
            'J'.code, 'K'.code -> {
                mAboutToAutoWrap = false
                val fillChar = ' '.code
                var startCol = -1
                var startRow = -1
                var endCol = -1
                var endRow = -1
                val justRow = (b == 'K'.code)
                when (getArg0(0)) {
                    0 -> {
                        startCol = cursorCol; startRow = cursorRow
                        endCol = mColumns; endRow = if (justRow) cursorRow + 1 else mRows
                    }
                    1 -> {
                        startCol = 0; startRow = if (justRow) cursorRow else 0
                        endCol = cursorCol + 1; endRow = cursorRow + 1
                    }
                    2 -> {
                        startCol = 0; startRow = if (justRow) cursorRow else 0
                        endCol = mColumns; endRow = if (justRow) cursorRow + 1 else mRows
                    }
                    else -> unknownSequence(b)
                }
                val style = style
                for (row in startRow until endRow) {
                    for (col in startCol until endCol) {
                        if ((TextStyle.decodeEffect(screen.getStyleAt(row, col)) and TextStyle.CHARACTER_ATTRIBUTE_PROTECTED) == 0)
                            screen.setChar(col, row, fillChar, style)
                    }
                }
            }
            'h'.code, 'l'.code -> {
                if (mArgIndex >= mArgs.size) mArgIndex = mArgs.size - 1
                for (i in 0..mArgIndex)
                    doDecSetOrReset(b == 'h'.code, mArgs[i])
            }
            'n'.code -> {
                finishSequence()
                return
            }
            'r'.code, 's'.code -> {
                if (mArgIndex >= mArgs.size) mArgIndex = mArgs.size - 1
                for (i in 0..mArgIndex) {
                    val externalBit = mArgs[i]
                    val internalBit = mapDecSetBitToInternalBit(externalBit)
                    if (internalBit == -1) {
                        Timber.tag(LOG_TAG).w("Ignoring request to save/recall decset bit=$externalBit")
                    } else {
                        if (b == 's'.code) {
                            mSavedDecSetFlags = mSavedDecSetFlags or internalBit
                        } else {
                            doDecSetOrReset((mSavedDecSetFlags and internalBit) != 0, externalBit)
                        }
                    }
                }
            }
            '$'.code -> {
                continueSequence(ESC_CSI_QUESTIONMARK_ARG_DOLLAR)
                return
            }
            else -> parseArg(b)
        }
    }

    fun doDecSetOrReset(setting: Boolean, externalBit: Int) {
        val internalBit = mapDecSetBitToInternalBit(externalBit)
        if (internalBit != -1) {
            setDecsetinternalBit(internalBit, setting)
        }
        when (externalBit) {
            1 -> { /* Application Cursor Keys (DECCKM) */ }
            3 -> {
                mLeftMargin = 0
                mTopMargin = 0
                mBottomMargin = mRows
                mRightMargin = mColumns
                setDecsetinternalBit(DECSET_BIT_LEFTRIGHT_MARGIN_MODE, false)
                blockClear(0, 0, mColumns, mRows)
                setCursorRowCol(0, 0)
            }
            4 -> { /* DECSCLM-Scrolling Mode. Ignore */ }
            5 -> { /* Reverse video. No action */ }
            6 -> if (setting) setCursorPosition(0, 0)
            7, 8, 9, 12, 25 -> { /* Cursor state change - ignored for read-only */ }
            40, 45, 66 -> { /* Ignore */ }
            69 -> {
                if (!setting) {
                    mLeftMargin = 0
                    mRightMargin = mColumns
                }
            }
            1000, 1001, 1002, 1003, 1004, 1005, 1006, 1015, 1034 -> { /* Ignore */ }
            1048 -> if (setting) saveCursor() else restoreCursor()
            47, 1047, 1049 -> {
                val newScreen = if (setting) mAltBuffer else mMainBuffer
                if (newScreen !== screen) {
                    val resized = !(newScreen.columns == mColumns && newScreen.screenRows == mRows)
                    if (setting) saveCursor()
                    screen = newScreen
                    if (!setting) {
                        val col = mSavedStateMain.mSavedCursorCol
                        val row = mSavedStateMain.mSavedCursorRow
                        restoreCursor()
                        if (resized) {
                            cursorCol = col
                            cursorRow = row
                        }
                    }
                    if (resized) resizeScreen()
                    if (newScreen === mAltBuffer)
                        newScreen.blockSet(0, 0, mColumns, mRows, ' '.code, style)
                }
            }
            2004 -> { /* Bracketed paste mode - setting bit is enough */ }
            else -> unknownParameter(externalBit)
        }
    }

    private fun doCsiBiggerThan(b: Int) {
        when (b) {
            'c'.code -> { /* Secondary device attributes - ignored for read-only */ }
            'm'.code -> Timber.tag(LOG_TAG).e("(ignored) CSI > MODIFY RESOURCE: ${getArg0(-1)} to ${getArg1(-1)}")
            else -> parseArg(b)
        }
    }

    private fun startEscapeSequence() {
        mEscapeState = ESC
        mArgIndex = 0
        mArgs.fill(-1)
        mArgsSubParamsBitSet = 0
    }

    private fun doLinefeed() {
        val belowScrollingRegion = cursorRow >= mBottomMargin
        var newCursorRow = cursorRow + 1
        if (belowScrollingRegion) {
            if (cursorRow != mRows - 1) {
                setCursorRow(newCursorRow)
            }
        } else {
            if (newCursorRow == mBottomMargin) {
                scrollDownOneLine()
                newCursorRow = mBottomMargin - 1
            }
            setCursorRow(newCursorRow)
        }
    }

    private fun continueSequence(state: Int) {
        mEscapeState = state
        mContinueSequence = true
    }

    private fun doEscPound(b: Int) {
        when (b) {
            '8'.code -> screen.blockSet(0, 0, mColumns, mRows, 'E'.code, style)
            else -> unknownSequence(b)
        }
    }

    private fun doEsc(b: Int) {
        when (b) {
            '#'.code -> continueSequence(ESC_POUND)
            '('.code -> continueSequence(ESC_SELECT_LEFT_PAREN)
            ')'.code -> continueSequence(ESC_SELECT_RIGHT_PAREN)
            '6'.code -> {
                if (cursorCol > mLeftMargin) {
                    cursorCol--
                } else {
                    val rows = mBottomMargin - mTopMargin
                    screen.blockCopy(mLeftMargin, mTopMargin, mRightMargin - mLeftMargin - 1, rows, mLeftMargin + 1, mTopMargin)
                    screen.blockSet(mLeftMargin, mTopMargin, 1, rows, ' '.code, TextStyle.encode(mForeColor, mBackColor, 0))
                }
            }
            '7'.code -> saveCursor()
            '8'.code -> restoreCursor()
            '9'.code -> {
                if (cursorCol < mRightMargin - 1) {
                    cursorCol++
                } else {
                    val rows = mBottomMargin - mTopMargin
                    screen.blockCopy(mLeftMargin + 1, mTopMargin, mRightMargin - mLeftMargin - 1, rows, mLeftMargin, mTopMargin)
                    screen.blockSet(mRightMargin - 1, mTopMargin, 1, rows, ' '.code, TextStyle.encode(mForeColor, mBackColor, 0))
                }
            }
            'c'.code -> {
                reset()
                mMainBuffer.clearTranscript()
                blockClear(0, 0, mColumns, mRows)
                setCursorPosition(0, 0)
            }
            'D'.code -> doLinefeed()
            'E'.code -> {
                setCursorCol(if (isDecsetInternalBitSet(DECSET_BIT_ORIGIN_MODE)) mLeftMargin else 0)
                doLinefeed()
            }
            'F'.code -> setCursorRowCol(0, mBottomMargin - 1)
            'H'.code -> mTabStop[cursorCol] = true
            'M'.code -> {
                if (cursorRow <= mTopMargin) {
                    screen.blockCopy(mLeftMargin, mTopMargin, mRightMargin - mLeftMargin, mBottomMargin - (mTopMargin + 1), mLeftMargin, mTopMargin + 1)
                    blockClear(mLeftMargin, mTopMargin, mRightMargin - mLeftMargin)
                } else {
                    cursorRow--
                }
            }
            'N'.code, '0'.code -> { /* SS2/SS3, ignore */ }
            'P'.code -> {
                mOSCOrDeviceControlArgs.clear()
                continueSequence(ESC_P)
            }
            '['.code -> continueSequence(ESC_CSI)
            '='.code -> setDecsetinternalBit(DECSET_BIT_APPLICATION_KEYPAD, true)
            ']'.code -> {
                mOSCOrDeviceControlArgs.clear()
                continueSequence(ESC_OSC)
            }
            '>'.code -> setDecsetinternalBit(DECSET_BIT_APPLICATION_KEYPAD, false)
            '_'.code -> continueSequence(ESC_APC)
            '%'.code -> continueSequence(ESC_PERCENT)
            else -> unknownSequence(b)
        }
    }

    private fun saveCursor() {
        val state = if (screen === mMainBuffer) mSavedStateMain else mSavedStateAlt
        state.mSavedCursorRow = cursorRow
        state.mSavedCursorCol = cursorCol
        state.mSavedEffect = mEffect
        state.mSavedForeColor = mForeColor
        state.mSavedBackColor = mBackColor
        state.mSavedDecFlags = mCurrentDecSetFlags
        state.mUseLineDrawingG0 = mUseLineDrawingG0
        state.mUseLineDrawingG1 = mUseLineDrawingG1
        state.mUseLineDrawingUsesG0 = mUseLineDrawingUsesG0
    }

    private fun restoreCursor() {
        val state = if (screen === mMainBuffer) mSavedStateMain else mSavedStateAlt
        setCursorRowCol(state.mSavedCursorRow, state.mSavedCursorCol)
        mEffect = state.mSavedEffect
        mForeColor = state.mSavedForeColor
        mBackColor = state.mSavedBackColor
        val mask = DECSET_BIT_AUTOWRAP or DECSET_BIT_ORIGIN_MODE
        mCurrentDecSetFlags = (mCurrentDecSetFlags and mask.inv()) or (state.mSavedDecFlags and mask)
        mUseLineDrawingG0 = state.mUseLineDrawingG0
        mUseLineDrawingG1 = state.mUseLineDrawingG1
        mUseLineDrawingUsesG0 = state.mUseLineDrawingUsesG0
    }

    private fun doCsi(b: Int) {
        when (b) {
            '!'.code -> continueSequence(ESC_CSI_EXCLAMATION)
            '"'.code -> continueSequence(ESC_CSI_DOUBLE_QUOTE)
            '\''.code -> continueSequence(ESC_CSI_SINGLE_QUOTE)
            '$'.code -> continueSequence(ESC_CSI_DOLLAR)
            '*'.code -> continueSequence(ESC_CSI_ARGS_ASTERIX)
            '@'.code -> {
                mAboutToAutoWrap = false
                val columnsAfterCursor = mColumns - cursorCol
                val spacesToInsert = minOf(getArg0(1), columnsAfterCursor)
                val charsToMove = columnsAfterCursor - spacesToInsert
                screen.blockCopy(cursorCol, cursorRow, charsToMove, 1, cursorCol + spacesToInsert, cursorRow)
                blockClear(cursorCol, cursorRow, spacesToInsert)
            }
            'A'.code -> setCursorRow(maxOf(0, cursorRow - getArg0(1)))
            'B'.code -> setCursorRow(minOf(mRows - 1, cursorRow + getArg0(1)))
            'C'.code, 'a'.code -> setCursorCol(minOf(mRightMargin - 1, cursorCol + getArg0(1)))
            'D'.code -> setCursorCol(maxOf(mLeftMargin, cursorCol - getArg0(1)))
            'E'.code -> setCursorPosition(0, cursorRow + getArg0(1))
            'F'.code -> setCursorPosition(0, cursorRow - getArg0(1))
            'G'.code -> setCursorCol(minOf(maxOf(1, getArg0(1)), mColumns) - 1)
            'H'.code, 'f'.code -> setCursorPosition(getArg1(1) - 1, getArg0(1) - 1)
            'I'.code -> setCursorCol(nextTabStop(getArg0(1)))
            'J'.code -> {
                when (getArg0(0)) {
                    0 -> {
                        blockClear(cursorCol, cursorRow, mColumns - cursorCol)
                        blockClear(0, cursorRow + 1, mColumns, mRows - (cursorRow + 1))
                    }
                    1 -> {
                        blockClear(0, 0, mColumns, cursorRow)
                        blockClear(0, cursorRow, cursorCol + 1)
                    }
                    2 -> blockClear(0, 0, mColumns, mRows)
                    3 -> mMainBuffer.clearTranscript()
                    else -> {
                        unknownSequence(b)
                        return
                    }
                }
                mAboutToAutoWrap = false
            }
            'K'.code -> {
                when (getArg0(0)) {
                    0 -> blockClear(cursorCol, cursorRow, mColumns - cursorCol)
                    1 -> blockClear(0, cursorRow, cursorCol + 1)
                    2 -> blockClear(0, cursorRow, mColumns)
                    else -> {
                        unknownSequence(b)
                        return
                    }
                }
                mAboutToAutoWrap = false
            }
            'L'.code -> {
                val linesAfterCursor = mBottomMargin - cursorRow
                val linesToInsert = minOf(getArg0(1), linesAfterCursor)
                val linesToMove = linesAfterCursor - linesToInsert
                screen.blockCopy(0, cursorRow, mColumns, linesToMove, 0, cursorRow + linesToInsert)
                blockClear(0, cursorRow, mColumns, linesToInsert)
            }
            'M'.code -> {
                mAboutToAutoWrap = false
                val linesAfterCursor = mBottomMargin - cursorRow
                val linesToDelete = minOf(getArg0(1), linesAfterCursor)
                val linesToMove = linesAfterCursor - linesToDelete
                screen.blockCopy(0, cursorRow + linesToDelete, mColumns, linesToMove, 0, cursorRow)
                blockClear(0, cursorRow + linesToMove, mColumns, linesToDelete)
            }
            'P'.code -> {
                mAboutToAutoWrap = false
                val cellsAfterCursor = mColumns - cursorCol
                val cellsToDelete = minOf(getArg0(1), cellsAfterCursor)
                val cellsToMove = cellsAfterCursor - cellsToDelete
                screen.blockCopy(cursorCol + cellsToDelete, cursorRow, cellsToMove, 1, cursorCol, cursorRow)
                blockClear(cursorCol + cellsToMove, cursorRow, cellsToDelete)
            }
            'S'.code -> {
                val linesToScroll = getArg0(1)
                for (i in 0 until linesToScroll)
                    scrollDownOneLine()
            }
            'T'.code -> {
                if (mArgIndex == 0) {
                    val linesToScrollArg = getArg0(1)
                    val linesBetweenTopAndBottomMargins = mBottomMargin - mTopMargin
                    val linesToScroll = minOf(linesBetweenTopAndBottomMargins, linesToScrollArg)
                    screen.blockCopy(mLeftMargin, mTopMargin, mRightMargin - mLeftMargin, linesBetweenTopAndBottomMargins - linesToScroll, mLeftMargin, mTopMargin + linesToScroll)
                    blockClear(mLeftMargin, mTopMargin, mRightMargin - mLeftMargin, linesToScroll)
                } else {
                    unimplementedSequence(b)
                }
            }
            'X'.code -> {
                mAboutToAutoWrap = false
                screen.blockSet(cursorCol, cursorRow, minOf(getArg0(1), mColumns - cursorCol), 1, ' '.code, style)
            }
            'Z'.code -> {
                var numberOfTabs = getArg0(1)
                var newCol = mLeftMargin
                for (i in cursorCol - 1 downTo 0)
                    if (mTabStop[i]) {
                        if (--numberOfTabs == 0) {
                            newCol = maxOf(i, mLeftMargin)
                            break
                        }
                    }
                cursorCol = newCol
            }
            '?'.code -> continueSequence(ESC_CSI_QUESTIONMARK)
            '>'.code -> continueSequence(ESC_CSI_BIGGERTHAN)
            '<'.code, '='.code -> continueSequence(ESC_CSI_UNSUPPORTED_PARAMETER_BYTE)
            '`'.code -> setCursorColRespectingOriginMode(getArg0(1) - 1)
            'b'.code -> {
                if (mLastEmittedCodePoint == -1) return
                val numRepeat = getArg0(1)
                for (i in 0 until numRepeat) emitCodePoint(mLastEmittedCodePoint)
            }
            'c'.code -> { /* Primary device attributes - ignored for read-only */ }
            'd'.code -> setCursorRow(minOf(maxOf(1, getArg0(1)), mRows) - 1)
            'e'.code -> setCursorPosition(cursorCol, cursorRow + getArg0(1))
            'g'.code -> {
                when (getArg0(0)) {
                    0 -> mTabStop[cursorCol] = false
                    3 -> for (i in 0 until mColumns) mTabStop[i] = false
                }
            }
            'h'.code -> doSetMode(true)
            'l'.code -> doSetMode(false)
            'm'.code -> selectGraphicRendition()
            'n'.code -> { /* Device Status Report - ignored for read-only */ }
            'r'.code -> {
                mTopMargin = maxOf(0, minOf(getArg0(1) - 1, mRows - 2))
                mBottomMargin = maxOf(mTopMargin + 2, minOf(getArg1(mRows), mRows))
                setCursorPosition(0, 0)
            }
            's'.code -> {
                if (isDecsetInternalBitSet(DECSET_BIT_LEFTRIGHT_MARGIN_MODE)) {
                    mLeftMargin = minOf(getArg0(1) - 1, mColumns - 2)
                    mRightMargin = maxOf(mLeftMargin + 1, minOf(getArg1(mColumns), mColumns))
                    setCursorPosition(0, 0)
                } else {
                    saveCursor()
                }
            }
            't'.code -> {
                when (getArg0(0)) {
                    22 -> {
                        titleStack.push(title)
                        if (titleStack.size > 20) {
                            titleStack.removeAt(0)
                        }
                    }
                    23 -> if (titleStack.isNotEmpty()) setTitle(titleStack.pop())
                }
            }
            'u'.code -> restoreCursor()
            ' '.code -> continueSequence(ESC_CSI_ARGS_SPACE)
            else -> parseArg(b)
        }
    }

    private fun selectGraphicRendition() {
        if (mArgIndex >= mArgs.size) mArgIndex = mArgs.size - 1
        var i = 0
        while (i <= mArgIndex) {
            if ((mArgsSubParamsBitSet and (1 shl i)) != 0) {
                i++
                continue
            }

            var code = getArg(i, 0, false)
            if (code < 0) {
                if (mArgIndex > 0) {
                    i++
                    continue
                } else {
                    code = 0
                }
            }
            when {
                code == 0 -> {
                    mForeColor = TextStyle.COLOR_INDEX_FOREGROUND
                    mBackColor = TextStyle.COLOR_INDEX_BACKGROUND
                    mEffect = 0
                }
                code == 1 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_BOLD
                code == 2 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_DIM
                code == 3 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_ITALIC
                code == 4 -> {
                    if (i + 1 <= mArgIndex && ((mArgsSubParamsBitSet and (1 shl (i + 1))) != 0)) {
                        i++
                        if (mArgs[i] == 0) {
                            mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE.inv()
                        } else {
                            mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE
                        }
                    } else {
                        mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE
                    }
                }
                code == 5 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_BLINK
                code == 7 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_INVERSE
                code == 8 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_INVISIBLE
                code == 9 -> mEffect = mEffect or TextStyle.CHARACTER_ATTRIBUTE_STRIKETHROUGH
                code == 10 -> { /* Exit alt charset (TERM=linux) - ignore */ }
                code == 11 -> { /* Enter alt charset (TERM=linux) - ignore */ }
                code == 22 -> mEffect = mEffect and (TextStyle.CHARACTER_ATTRIBUTE_BOLD or TextStyle.CHARACTER_ATTRIBUTE_DIM).inv()
                code == 23 -> mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_ITALIC.inv()
                code == 24 -> mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_UNDERLINE.inv()
                code == 25 -> mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_BLINK.inv()
                code == 27 -> mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_INVERSE.inv()
                code == 28 -> mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_INVISIBLE.inv()
                code == 29 -> mEffect = mEffect and TextStyle.CHARACTER_ATTRIBUTE_STRIKETHROUGH.inv()
                code in 30..37 -> mForeColor = code - 30
                code == 38 || code == 48 || code == 58 -> {
                    if (i + 2 > mArgIndex) { i++; continue }
                    val firstArg = mArgs[i + 1]
                    if (firstArg == 2) {
                        if (i + 4 > mArgIndex) {
                            Timber.tag(LOG_TAG).w("Too few CSI${code};2 RGB arguments")
                        } else {
                            val red = getArg(i + 2, 0, false)
                            val green = getArg(i + 3, 0, false)
                            val blue = getArg(i + 4, 0, false)
                            if (red < 0 || green < 0 || blue < 0 || red > 255 || green > 255 || blue > 255) {
                                finishSequenceAndLogError("Invalid RGB: $red,$green,$blue")
                            } else {
                                val argbColor = 0xff000000.toInt() or (red shl 16) or (green shl 8) or blue
                                when (code) {
                                    38 -> mForeColor = argbColor
                                    48 -> mBackColor = argbColor
                                    58 -> mUnderlineColor = argbColor
                                }
                            }
                            i += 4
                        }
                    } else if (firstArg == 5) {
                        val color = getArg(i + 2, 0, false)
                        i += 2
                        if (color in 0 until TextStyle.NUM_INDEXED_COLORS) {
                            when (code) {
                                38 -> mForeColor = color
                                48 -> mBackColor = color
                                58 -> mUnderlineColor = color
                            }
                        } else {
                            if (LOG_ESCAPE_SEQUENCES) Timber.tag(LOG_TAG).w("Invalid color index: $color")
                        }
                    } else {
                        finishSequenceAndLogError("Invalid ISO-8613-3 SGR first argument: $firstArg")
                    }
                }
                code == 39 -> mForeColor = TextStyle.COLOR_INDEX_FOREGROUND
                code in 40..47 -> mBackColor = code - 40
                code == 49 -> mBackColor = TextStyle.COLOR_INDEX_BACKGROUND
                code == 59 -> mUnderlineColor = TextStyle.COLOR_INDEX_FOREGROUND
                code in 90..97 -> mForeColor = code - 90 + 8
                code in 100..107 -> mBackColor = code - 100 + 8
                else -> {
                    if (LOG_ESCAPE_SEQUENCES)
                        Timber.tag(LOG_TAG).w("SGR unknown code %d", code)
                }
            }
            i++
        }
    }

    private fun doOsc(b: Int) {
        when (b) {
            7 -> doOscSetTextParameters("\u0007")
            27 -> continueSequence(ESC_OSC_ESC)
            else -> collectOSCArgs(b)
        }
    }

    private fun doOscEsc(b: Int) {
        when (b) {
            '\\'.code -> doOscSetTextParameters("\u001b\\")
            else -> {
                collectOSCArgs(27)
                collectOSCArgs(b)
                continueSequence(ESC_OSC)
            }
        }
    }

    private fun doOscSetTextParameters(bellOrStringTerminator: String) {
        var value = -1
        var textParameter = ""
        for (idx in 0 until mOSCOrDeviceControlArgs.length) {
            val b = mOSCOrDeviceControlArgs[idx]
            if (b == ';') {
                textParameter = mOSCOrDeviceControlArgs.substring(idx + 1)
                break
            } else if (b in '0'..'9') {
                value = (if (value < 0) 0 else value * 10) + (b - '0')
            } else {
                unknownSequence(b.code)
                return
            }
        }

        when (value) {
            0, 1, 2 -> setTitle(textParameter)
            4 -> {
                var colorIndex = -1
                var parsingPairStart = -1
                var i = 0
                while (true) {
                    val endOfInput = i == textParameter.length
                    val ch = if (endOfInput) ';' else textParameter[i]
                    if (ch == ';') {
                        if (parsingPairStart < 0) {
                            parsingPairStart = i + 1
                        } else {
                            if (colorIndex < 0 || colorIndex > 255) {
                                unknownSequence(ch.code)
                                return
                            } else {
                                mColors.tryParseColor(colorIndex, textParameter.substring(parsingPairStart, i))
                                colorIndex = -1
                                parsingPairStart = -1
                            }
                        }
                    } else if (parsingPairStart >= 0) {
                        // Passing through color spec
                    } else if (parsingPairStart < 0 && ch in '0'..'9') {
                        colorIndex = (if (colorIndex < 0) 0 else colorIndex * 10) + (ch - '0')
                    } else {
                        unknownSequence(ch.code)
                        return
                    }
                    if (endOfInput) break
                    i++
                }
            }
            10, 11, 12 -> {
                var specialIndex = TextStyle.COLOR_INDEX_FOREGROUND + (value - 10)
                var lastSemiIndex = 0
                var charIndex = 0
                while (true) {
                    val endOfInput = charIndex == textParameter.length
                    if (endOfInput || textParameter[charIndex] == ';') {
                        try {
                            val colorSpec = textParameter.substring(lastSemiIndex, charIndex)
                            if ("?" != colorSpec) {
                                mColors.tryParseColor(specialIndex, colorSpec)
                            }
                            specialIndex++
                            charIndex++
                            if (endOfInput || specialIndex > TextStyle.COLOR_INDEX_CURSOR || charIndex >= textParameter.length)
                                break
                            lastSemiIndex = charIndex
                        } catch (_: NumberFormatException) {
                            // Ignore
                        }
                    }
                    charIndex++
                }
            }
            52 -> {
                val startIndex = textParameter.indexOf(";") + 1
                try {
                    val clipboardText = String(Base64.decode(textParameter.substring(startIndex), 0), Charsets.UTF_8)
                    onCopyToClipboard?.invoke(clipboardText)
                } catch (_: Exception) {
                    Timber.tag(LOG_TAG).e("OSC Manipulate selection, invalid string '$textParameter'")
                }
            }
            104 -> {
                if (textParameter.isEmpty()) {
                    mColors.reset()
                } else {
                    var lastIndex = 0
                    var charIndex = 0
                    while (true) {
                        val endOfInput = charIndex == textParameter.length
                        if (endOfInput || textParameter[charIndex] == ';') {
                            try {
                                val colorToReset = textParameter.substring(lastIndex, charIndex).toInt()
                                mColors.reset(colorToReset)
                                if (endOfInput) break
                                charIndex++
                                lastIndex = charIndex
                            } catch (_: NumberFormatException) {
                                // Ignore
                            }
                        }
                        charIndex++
                    }
                }
            }
            110, 111, 112 -> {
                mColors.reset(TextStyle.COLOR_INDEX_FOREGROUND + (value - 110))
            }
            119 -> { /* Reset highlight color - ignore */ }
            else -> unknownParameter(value)
        }
        finishSequence()
    }

    private fun blockClear(sx: Int, sy: Int, w: Int) {
        blockClear(sx, sy, w, 1)
    }

    private fun blockClear(sx: Int, sy: Int, w: Int, h: Int) {
        screen.blockSet(sx, sy, w, h, ' '.code, style)
    }

    private val style: Long get() = TextStyle.encode(mForeColor, mBackColor, mEffect)

    private fun doSetMode(newValue: Boolean) {
        val modeBit = getArg0(0)
        when (modeBit) {
            4 -> mInsertMode = newValue
            20 -> unknownParameter(modeBit)
            34 -> { /* Normal cursor visibility - ignore */ }
            else -> unknownParameter(modeBit)
        }
    }

    private fun setCursorPosition(x: Int, y: Int) {
        val originMode = isDecsetInternalBitSet(DECSET_BIT_ORIGIN_MODE)
        val effectiveTopMargin = if (originMode) mTopMargin else 0
        val effectiveBottomMargin = if (originMode) mBottomMargin else mRows
        val effectiveLeftMargin = if (originMode) mLeftMargin else 0
        val effectiveRightMargin = if (originMode) mRightMargin else mColumns
        val newRow = maxOf(effectiveTopMargin, minOf(effectiveTopMargin + y, effectiveBottomMargin - 1))
        val newCol = maxOf(effectiveLeftMargin, minOf(effectiveLeftMargin + x, effectiveRightMargin - 1))
        setCursorRowCol(newRow, newCol)
    }

    private fun scrollDownOneLine() {
        scrollCounter++
        val currentStyle = style
        if (mLeftMargin != 0 || mRightMargin != mColumns) {
            screen.blockCopy(mLeftMargin, mTopMargin + 1, mRightMargin - mLeftMargin, mBottomMargin - mTopMargin - 1, mLeftMargin, mTopMargin)
            screen.blockSet(mLeftMargin, mBottomMargin - 1, mRightMargin - mLeftMargin, 1, ' '.code, currentStyle)
        } else {
            screen.scrollDownOneLine(mTopMargin, mBottomMargin, currentStyle)
        }
    }

    private fun parseArg(b: Int) {
        if (b >= '0'.code && b <= '9'.code) {
            if (mArgIndex < mArgs.size) {
                val oldValue = mArgs[mArgIndex]
                val thisDigit = b - '0'.code
                val value: Int = if (oldValue >= 0) {
                    oldValue * 10 + thisDigit
                } else {
                    thisDigit
                }
                mArgs[mArgIndex] = if (value > 9999) 9999 else value
            }
            continueSequence(mEscapeState)
        } else if (b == ';'.code || b == ':'.code) {
            if (mArgIndex + 1 < mArgs.size) {
                mArgIndex++
                if (b == ':'.code) {
                    mArgsSubParamsBitSet = mArgsSubParamsBitSet or (1 shl mArgIndex)
                }
            } else {
                logError("Too many parameters when in state: $mEscapeState")
            }
            continueSequence(mEscapeState)
        } else {
            unknownSequence(b)
        }
    }

    private fun getArg0(defaultValue: Int): Int = getArg(0, defaultValue, true)

    private fun getArg1(defaultValue: Int): Int = getArg(1, defaultValue, true)

    private fun getArg(index: Int, defaultValue: Int, treatZeroAsDefault: Boolean): Int {
        var result = mArgs[index]
        if (result < 0 || (result == 0 && treatZeroAsDefault)) {
            result = defaultValue
        }
        return result
    }

    private fun collectOSCArgs(b: Int) {
        if (mOSCOrDeviceControlArgs.length < MAX_OSC_STRING_LENGTH) {
            mOSCOrDeviceControlArgs.appendCodePoint(b)
            continueSequence(mEscapeState)
        } else {
            unknownSequence(b)
        }
    }

    private fun unimplementedSequence(b: Int) {
        logError("Unimplemented sequence char '${b.toChar()}' (U+${String.format("%04x", b)})")
        finishSequence()
    }

    private fun unknownSequence(b: Int) {
        logError("Unknown sequence char '${b.toChar()}' (numeric value=$b)")
        finishSequence()
    }

    private fun unknownParameter(parameter: Int) {
        logError("Unknown parameter: $parameter")
        finishSequence()
    }

    private fun logError(errorType: String) {
        if (LOG_ESCAPE_SEQUENCES) {
            val buf = StringBuilder()
            buf.append(errorType)
            buf.append(", escapeState=")
            buf.append(mEscapeState)
            var firstArg = true
            if (mArgIndex >= mArgs.size) mArgIndex = mArgs.size - 1
            for (i in 0..mArgIndex) {
                val value = mArgs[i]
                if (value >= 0) {
                    if (firstArg) {
                        firstArg = false
                        buf.append(", args={")
                    } else {
                        buf.append(',')
                    }
                    buf.append(value)
                }
            }
            if (!firstArg) buf.append('}')
            finishSequenceAndLogError(buf.toString())
        }
    }

    private fun finishSequenceAndLogError(error: String) {
        if (LOG_ESCAPE_SEQUENCES) Timber.tag(LOG_TAG).w(error)
        finishSequence()
    }

    private fun finishSequence() {
        mEscapeState = ESC_NONE
    }

    private fun emitCodePoint(codePoint: Int) {
        var cp = codePoint
        mLastEmittedCodePoint = cp
        if (if (mUseLineDrawingUsesG0) mUseLineDrawingG0 else mUseLineDrawingG1) {
            when (cp) {
                '_'.code -> cp = ' '.code
                '`'.code -> cp = '\u25C6'.code // Diamond
                '0'.code -> cp = '\u2588'.code // Solid block
                'a'.code -> cp = '\u2592'.code // Checker board
                'b'.code -> cp = '\u2409'.code // Horizontal tab
                'c'.code -> cp = '\u240C'.code // Form feed
                'd'.code -> cp = '\r'.code     // Carriage return
                'e'.code -> cp = '\u240A'.code // Linefeed
                'f'.code -> cp = '\u00B0'.code // Degree
                'g'.code -> cp = '\u00B1'.code // Plus-minus
                'h'.code -> cp = '\n'.code     // Newline
                'i'.code -> cp = '\u240B'.code // Vertical tab
                'j'.code -> cp = '\u2518'.code // Lower right corner
                'k'.code -> cp = '\u2510'.code // Upper right corner
                'l'.code -> cp = '\u250C'.code // Upper left corner
                'm'.code -> cp = '\u2514'.code // Lower left corner
                'n'.code -> cp = '\u253C'.code // Crossing lines
                'o'.code -> cp = '\u23BA'.code // Horizontal line - scan 1
                'p'.code -> cp = '\u23BB'.code // Horizontal line - scan 3
                'q'.code -> cp = '\u2500'.code // Horizontal line - scan 5
                'r'.code -> cp = '\u23BC'.code // Horizontal line - scan 7
                's'.code -> cp = '\u23BD'.code // Horizontal line - scan 9
                't'.code -> cp = '\u251C'.code // T facing rightwards
                'u'.code -> cp = '\u2524'.code // T facing leftwards
                'v'.code -> cp = '\u2534'.code // T facing upwards
                'w'.code -> cp = '\u252C'.code // T facing downwards
                'x'.code -> cp = '\u2502'.code // Vertical line
                'y'.code -> cp = '\u2264'.code // Less than or equal to
                'z'.code -> cp = '\u2265'.code // Greater than or equal to
                '{'.code -> cp = '\u03C0'.code // Pi
                '|'.code -> cp = '\u2260'.code // Not equal to
                '}'.code -> cp = '\u00A3'.code // UK pound
                '~'.code -> cp = '\u00B7'.code // Centered dot
            }
        }

        val autoWrap = isDecsetInternalBitSet(DECSET_BIT_AUTOWRAP)
        val displayWidth = WcWidth.width(cp)
        val cursorInLastColumn = cursorCol == mRightMargin - 1

        if (autoWrap) {
            if (cursorInLastColumn && ((mAboutToAutoWrap && displayWidth == 1) || displayWidth == 2)) {
                screen.setLineWrap(cursorRow)
                cursorCol = mLeftMargin
                if (cursorRow + 1 < mBottomMargin) {
                    cursorRow++
                } else {
                    scrollDownOneLine()
                }
            }
        } else if (cursorInLastColumn && displayWidth == 2) {
            return
        }

        if (mInsertMode && displayWidth > 0) {
            val destCol = cursorCol + displayWidth
            if (destCol < mRightMargin)
                screen.blockCopy(cursorCol, cursorRow, mRightMargin - destCol, 1, destCol, cursorRow)
        }

        val offsetDueToCombiningChar = if (displayWidth <= 0 && cursorCol > 0 && !mAboutToAutoWrap) 1 else 0
        var column = cursorCol - offsetDueToCombiningChar

        if (column < 0) column = 0
        screen.setChar(column, cursorRow, cp, style)

        if (autoWrap && displayWidth > 0)
            mAboutToAutoWrap = (cursorCol == mRightMargin - displayWidth)

        cursorCol = minOf(cursorCol + displayWidth, mRightMargin - 1)
    }

    private fun setCursorRow(row: Int) {
        cursorRow = row
        mAboutToAutoWrap = false
    }

    private fun setCursorCol(col: Int) {
        cursorCol = col
        mAboutToAutoWrap = false
    }

    private fun setCursorColRespectingOriginMode(col: Int) {
        setCursorPosition(col, cursorRow)
    }

    private fun setCursorRowCol(row: Int, col: Int) {
        cursorRow = maxOf(0, minOf(row, mRows - 1))
        cursorCol = maxOf(0, minOf(col, mColumns - 1))
        mAboutToAutoWrap = false
    }

    fun clearScrollCounter() {
        scrollCounter = 0
    }

    fun toggleAutoScrollDisabled() {
        isAutoScrollDisabled = !isAutoScrollDisabled
    }

    fun reset() {
        setCursorStyle()
        mArgIndex = 0
        mContinueSequence = false
        mEscapeState = ESC_NONE
        mInsertMode = false
        mTopMargin = 0
        mLeftMargin = 0
        mBottomMargin = mRows
        mRightMargin = mColumns
        mAboutToAutoWrap = false
        mForeColor = TextStyle.COLOR_INDEX_FOREGROUND
        mSavedStateMain.mSavedForeColor = TextStyle.COLOR_INDEX_FOREGROUND
        mSavedStateAlt.mSavedForeColor = TextStyle.COLOR_INDEX_FOREGROUND
        mBackColor = TextStyle.COLOR_INDEX_BACKGROUND
        mSavedStateMain.mSavedBackColor = TextStyle.COLOR_INDEX_BACKGROUND
        mSavedStateAlt.mSavedBackColor = TextStyle.COLOR_INDEX_BACKGROUND
        setDefaultTabStops()

        mUseLineDrawingG0 = false
        mUseLineDrawingG1 = false
        mUseLineDrawingUsesG0 = true

        mSavedStateMain.mSavedCursorRow = 0
        mSavedStateMain.mSavedCursorCol = 0
        mSavedStateMain.mSavedEffect = 0
        mSavedStateMain.mSavedDecFlags = 0
        mSavedStateAlt.mSavedCursorRow = 0
        mSavedStateAlt.mSavedCursorCol = 0
        mSavedStateAlt.mSavedEffect = 0
        mSavedStateAlt.mSavedDecFlags = 0
        mCurrentDecSetFlags = 0
        setDecsetinternalBit(DECSET_BIT_AUTOWRAP, true)
        setDecsetinternalBit(DECSET_BIT_CURSOR_ENABLED, true)
        mSavedDecSetFlags = mCurrentDecSetFlags
        mSavedStateMain.mSavedDecFlags = mCurrentDecSetFlags
        mSavedStateAlt.mSavedDecFlags = mCurrentDecSetFlags

        mUtf8Index = 0
        mUtf8ToFollow = 0

        mColors.reset()
    }

    fun getSelectedText(x1: Int, y1: Int, x2: Int, y2: Int): String =
        screen.getSelectedText(x1, y1, x2, y2)

    private fun setTitle(newTitle: String?) {
        title = newTitle
    }

    internal class SavedScreenState {
        var mSavedCursorRow = 0
        var mSavedCursorCol = 0
        var mSavedEffect = 0
        var mSavedForeColor = 0
        var mSavedBackColor = 0
        var mSavedDecFlags = 0
        var mUseLineDrawingG0 = false
        var mUseLineDrawingG1 = false
        var mUseLineDrawingUsesG0 = true
    }

    override fun toString(): String =
        "TerminalEmulator[size=${screen.columns}x${screen.screenRows}, margins={$mTopMargin,$mRightMargin,$mBottomMargin,$mLeftMargin}]"
}
