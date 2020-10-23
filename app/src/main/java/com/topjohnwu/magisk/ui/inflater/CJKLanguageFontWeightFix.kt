package com.topjohnwu.magisk.ui.inflater

import android.content.Context
import android.graphics.Typeface
import android.os.Build
import android.util.AttributeSet
import android.util.Xml
import android.view.View
import android.widget.TextView
import androidx.appcompat.widget.Toolbar
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.utils.currentLocale
import com.topjohnwu.magisk.ktx.now
import org.xmlpull.v1.XmlPullParser
import org.xmlpull.v1.XmlPullParserException
import timber.log.Timber
import java.io.EOFException
import java.io.FileInputStream
import java.io.IOException
import java.io.InputStream

/**
 * In AOSP, fonts for CJK languages have only regular weight, makes them
 * unable to display medium weight.<p>
 * If the font does not exists (check by parse fonts.xml), use bold for
 * them instead.
 */
object CJKLanguageFontWeightFix {

    private val TYPEFACE_MEDIUM = Typeface.create("sans-serif-medium", Typeface.NORMAL)
    private val TYPEFACE_BOLD = Typeface.create("sans-serif", Typeface.BOLD)

    enum class LANG(val index: Int) {
        ZH_HANS(0),
        ZH_HANT(1),
        JA(2),
        KO(3)
    }

    private val isMediumWeightExists by lazy(LazyThreadSafetyMode.NONE) {
        val array = BooleanArray(LANG.values().size) { false }

        //val current = now

        val fontConfig = try {
            FontListParser.parse(FileInputStream("/system/etc/fonts.xml"))
        } catch (e: Exception) {
            return@lazy array
        }

        fontConfig.families.forEach {
            val lang = when (it.lang) {
                "zh-Hans" -> LANG.ZH_HANS
                "zh-Hant", "zh-Hant,zh-Bopo" -> LANG.ZH_HANS
                "ja" -> LANG.JA
                "ko" -> LANG.KO
                else -> return@forEach
            }
            array[lang.index] = it.fonts.find { it.weight == 500 } != null
        }

        //Timber.d("Parse and filter fonts.xml in ${now - current}ms")
        array
    }

    fun onViewCreated(view: View, name: String, context: Context, attrs: AttributeSet) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) return

        val language = currentLocale.language
        if (language != "zh" && language != "ja" && language != "ko") return

        val country = currentLocale.country
        val lang = when {
            language == "zh" && country == "CN" -> LANG.ZH_HANS
            language == "zh" -> LANG.ZH_HANT
            language == "ja" -> LANG.JA
            language == "ko" -> LANG.KO
            else -> return
        }

        if (isMediumWeightExists[lang.index]) return

        if (view is TextView) {
            Timber.i(view.text.toString())

            // Typeface class have internal cache, so we can compare it with equals
            if (view.typeface == TYPEFACE_MEDIUM) {
                view.typeface = TYPEFACE_BOLD
            }
        } else if (view is Toolbar) {
            val a = view.context.obtainStyledAttributes(attrs, R.styleable.Toolbar, 0, 0)
            val titleTextAppearance = a.getResourceId(R.styleable.Toolbar_titleTextAppearance, 0)
            a.recycle()

            if (titleTextAppearance == R.style.AppearanceFoundation_Title) {
                view.setTitleTextAppearance(context, R.style.AppearanceFoundation_Title_Bold)
            }
        }
    }
}

private class FontConfig(
    val families: List<Family>,
    val aliases: List<Alias>
) {

    class Family(
        val name: String?,
        val fonts: List<Font>,
        val lang: String?,
        val variant: Int
    ) {

        companion object {
            const val VARIANT_DEFAULT = 0
            const val VARIANT_COMPACT = 1
            const val VARIANT_ELEGANT = 2
        }
    }

    class Font(
        val filename: String,
        val index: Int,
        val axes: List<FontVariationAxis>,
        val weight: Int,
        val isItalic: Boolean,
        val fallbackFor: String?
    )

    class Alias(
        val name: String?,
        val toName: String?,
        val weight: Int
    )

    class FontVariationAxis(
        val tag: String?,
        val stylevalue: String?
    )
}

private object FontListParser {

    @Throws(XmlPullParserException::class, IOException::class)
    fun parse(`in`: InputStream): FontConfig {
        return parse(`in`, "/system/fonts")
    }

    @Throws(XmlPullParserException::class, IOException::class)
    fun parse(stream: InputStream, fontDir: String): FontConfig {
        return stream.use {
            val parser: XmlPullParser = Xml.newPullParser()
            parser.setInput(stream, null)
            parser.nextTag()
            readFamilies(parser, fontDir)
        }
    }

    @Throws(XmlPullParserException::class, IOException::class)
    private fun readFamilies(parser: XmlPullParser, fontDir: String): FontConfig {
        val families = mutableListOf<FontConfig.Family>()
        val aliases = mutableListOf<FontConfig.Alias>()
        parser.require(XmlPullParser.START_TAG, null, "familyset")
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.eventType != XmlPullParser.START_TAG) continue
            when (parser.name) {
                "family" -> {
                    families.add(readFamily(parser, fontDir))
                }
                "alias" -> {
                    aliases.add(readAlias(parser))
                }
                else -> {
                    skip(parser)
                }
            }
        }
        return FontConfig(families, aliases)
    }

    @Throws(XmlPullParserException::class, IOException::class)
    fun readFamily(parser: XmlPullParser, fontDir: String): FontConfig.Family {
        val name = parser.getAttributeValue(null, "name")
        val lang = parser.getAttributeValue("", "lang")
        val variant = parser.getAttributeValue(null, "variant")
        val fonts = mutableListOf<FontConfig.Font>()
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.eventType != XmlPullParser.START_TAG) continue
            if (parser.name == "font") {
                fonts.add(readFont(parser, fontDir))
            } else {
                skip(parser)
            }
        }
        var intVariant = FontConfig.Family.VARIANT_DEFAULT
        if (variant != null) {
            if (variant == "compact") {
                intVariant = FontConfig.Family.VARIANT_COMPACT
            } else if (variant == "elegant") {
                intVariant = FontConfig.Family.VARIANT_ELEGANT
            }
        }
        return FontConfig.Family(name, fonts, lang, intVariant)
    }

    //private val FILENAME_WHITESPACE_PATTERN: Pattern = Pattern.compile("^[ \\n\\r\\t]+|[ \\n\\r\\t]+$")

    @Throws(XmlPullParserException::class, IOException::class)
    private fun readFont(parser: XmlPullParser, fontDir: String): FontConfig.Font {
        val index = parser.getAttributeValue(null, "index")?.toInt() ?: 0
        val axes = mutableListOf<FontConfig.FontVariationAxis>()
        val weight = parser.getAttributeValue(null, "weight")?.toInt() ?: 400
        val isItalic = "italic" == parser.getAttributeValue(null, "style")
        val fallbackFor = parser.getAttributeValue(null, "fallbackFor")
        val filename = StringBuilder()
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.eventType == XmlPullParser.TEXT) {
                filename.append(parser.text)
            }
            if (parser.eventType != XmlPullParser.START_TAG) continue
            if (parser.name == "axis") {
                axes.add(readAxis(parser))
            } else {
                skip(parser)
            }
        }
        //val sanitizedName = FILENAME_WHITESPACE_PATTERN.matcher(filename).replaceAll("")
        val sanitizedName = filename
        return FontConfig.Font(fontDir + sanitizedName, index, axes, weight, isItalic, fallbackFor)
    }

    @Throws(XmlPullParserException::class, IOException::class)
    private fun readAxis(parser: XmlPullParser): FontConfig.FontVariationAxis {
        val tagStr = parser.getAttributeValue(null, "tag")
        val styleValueStr = parser.getAttributeValue(null, "stylevalue")
        skip(parser) // axis tag is empty, ignore any contents and consume end tag
        return FontConfig.FontVariationAxis(tagStr, styleValueStr)
    }

    @Throws(XmlPullParserException::class, IOException::class)
    fun readAlias(parser: XmlPullParser): FontConfig.Alias {
        val name = parser.getAttributeValue(null, "name")
        val toName = parser.getAttributeValue(null, "to")
        val weight: Int = parser.getAttributeValue(null, "weight")?.toInt() ?: 400
        skip(parser) // alias tag is empty, ignore any contents and consume end tag
        return FontConfig.Alias(name, toName, weight)
    }

    @Throws(XmlPullParserException::class, IOException::class)
    fun skip(parser: XmlPullParser) {
        var depth = 1
        while (depth > 0) {
            when (parser.next()) {
                XmlPullParser.START_TAG -> depth++
                XmlPullParser.END_TAG -> depth--
                XmlPullParser.END_DOCUMENT -> throw EOFException("EOF")
            }
        }
    }
}