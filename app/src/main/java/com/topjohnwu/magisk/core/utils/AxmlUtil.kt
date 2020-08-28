package com.topjohnwu.magisk.core.utils

import java.io.ByteArrayOutputStream
import java.nio.charset.Charset
import java.util.*

private val UTF_16LE = Charset.forName("UTF-16LE")
private const val AXML_STRING_INDEXES_OFFSET = 0x20
private const val AXML_CHUNK_SIZE_OFFSET = 0x04


/** Hold a [ByteArray] reference that can be replaced */
class ByteArrayPtr(var ptr: ByteArray)

/** transform a byte to it's unsigned [Int] value */
private fun Byte.getValue(): Int = when {
    (this.toInt() < 0) -> this.toInt() + 256 else -> this.toInt()
}

/** this represent an index translation map */
class IndexMap {
    private val indexes = LinkedList<IntArray>();

    fun put(oldIndex: Int, newIndex: Int) {
        indexes.add(intArrayOf(oldIndex , newIndex - oldIndex))
    }

    fun translate(index: Int): Int {
        var indexDiff = 0
        for (entry in indexes.iterator()) {
            if (entry[0] > index) break
            indexDiff = entry[1]
        }
        return index + indexDiff
    }
}


/**
 * the "vararg ext: Int" parm is used to patch string that have different length
 * for example to path the manifest app id the arg 9 should be added to patch
 * "com.topjohnwu.magisk.provider" to "xxx.provider" since the length of the
 * string ".provider" is 9 charters
 * */
fun findAndPatchAXML(xml: ByteArrayPtr, from: CharSequence, to: CharSequence,vararg ext: Int): Boolean {
    /*if (from.length == to.length) {
        // If they are the same length just use the old hex patch method because it's faster
        return findAndPatch(xml.ptr, from, to)
    }*/
    var patched = findAndPatchAXMLRaw(xml,
            convertToAXML(from.toString(), 0),
            convertToAXML(to.toString(), 0))
    for (i in ext) {
        if (findAndPatchAXMLRaw(xml,
                        convertToAXML(from.toString(), i),
                        convertToAXML(to.toString(), i))) {
            patched = true
        }
    }
    return patched
}

/** allow to hex patch but with two array with different sizes
 * work similarly to the [String.replace] function
 */
private fun findAndPatchAXMLRaw(xmlPtr: ByteArrayPtr, from: ByteArray, to: ByteArray): Boolean {
    val xml = xmlPtr.ptr
    if (xml.size < from.size) return false
    val baos = ByteArrayOutputStream(xml.size)
    val indexMap = IndexMap()
    val stringChunkStart = findAXMLStringChunkOffset(xml)
    val origStringChunkEnd = stringChunkStart +
            getAXMLInt(xml, stringChunkStart + AXML_CHUNK_SIZE_OFFSET)
    var start = 0
    loop@ for(i in (stringChunkStart) until (origStringChunkEnd - from.size)) {
        if (i < start) continue@loop
        for(j in from.indices) {
            if (xml[i + j] != from[j]) {
                continue@loop
            }
        }
        baos.write(xml, start, i - start)
        baos.write(to, 0, to.size)
        start = i + from.size
        indexMap.put(start, baos.size())
    }
    if (start == 0) {
        // Don't waste effort on that
        return false
    }
    // Align bytes to x4
    val decl = (baos.size() - start) and 0x03
    if (decl != 0) {
        var deflate = true
        for (i in (decl-4) until 0) {
            if (xml[origStringChunkEnd + i] != 0.toByte()) {
                deflate = false
                break
            }
        }
        if (deflate) {
            baos.write(xml, start, origStringChunkEnd - start - (4-decl))
        } else {
            baos.write(xml, start, origStringChunkEnd - start)
            for (i in 0 until decl) {
                baos.write(0x00)
            }
        }
        indexMap.put(origStringChunkEnd, baos.size())
        start = origStringChunkEnd
    }
    // Write the end of the file
    baos.write(xml, start, xml.size - start)
    // Recalculate AXML size
    val newAXML = baos.toByteArray()
    val sizeDiff = newAXML.size - xml.size
    for (i in intArrayOf(AXML_CHUNK_SIZE_OFFSET, stringChunkStart + AXML_CHUNK_SIZE_OFFSET)) {
        setAXMLInt(newAXML, i, getAXMLInt(newAXML, i) + sizeDiff)
    }
    // Recalculate AXML entries indexes
    val stringIndexesStart = stringChunkStart + AXML_STRING_INDEXES_OFFSET
    var startResIndex = stringIndexesStart
    while (getAXMLShort(newAXML, startResIndex + 2) == 0) {
        startResIndex += 4
    }
    for(i in stringIndexesStart until startResIndex step 4) {
        // Translate old index to new index with startResIndex as diff
        setAXMLShort(newAXML, i,
                indexMap.translate(
                        getAXMLShort(newAXML, i) + startResIndex)
                        - startResIndex)
    }

    xmlPtr.ptr = newAXML
    return true
}

/** Convert a string to a native AXML string representation
 * The "ext: Int" argument allow to extend the length of the
 * resulting in a partial representation
 */
private fun convertToAXML(text: String, ext: Int): ByteArray {
    val length = text.length + ext
    return byteArrayOf((length and 0xFF).toByte(),
            (length shr 8 and 0xFF).toByte()) + text.toByteArray(UTF_16LE)
}

/** Find the AXML String chunk offset */
private fun findAXMLStringChunkOffset(axml: ByteArray): Int {
    var offset = 8
    while (offset < axml.size) {
        if (getAXMLInt(axml, offset) == 0x001C0001) {
            return offset
        }
        offset += getAXMLInt(axml, offset + AXML_CHUNK_SIZE_OFFSET)
    }
    throw Error("couldn't find the string chunk section")
}

private fun getAXMLShort(axml: ByteArray, index: Int): Int {
    return (axml[index].getValue()) or
            (axml[index + 1].getValue() shl 8)
}

private fun getAXMLInt(axml: ByteArray, index: Int): Int {
    return (axml[index].getValue()) or
            (axml[index + 1].getValue() shl 8) or
            (axml[index + 2].getValue() shl 16) or
            (axml[index + 3].getValue() shl 24)
}

private fun setAXMLShort(axml: ByteArray, index: Int, axmlInt: Int) {
    axml[index] = (axmlInt and 0xFF).toByte()
    axml[index + 1] = (axmlInt shr 8 and 0xFF).toByte()
}

private fun setAXMLInt(axml: ByteArray, index: Int, axmlInt: Int) {
    axml[index] = (axmlInt and 0xFF).toByte()
    axml[index + 1] = (axmlInt shr 8 and 0xFF).toByte()
    axml[index + 2] = (axmlInt shr 16 and 0xFF).toByte()
    axml[index + 3] = (axmlInt shr 24 and 0xFF).toByte()
}