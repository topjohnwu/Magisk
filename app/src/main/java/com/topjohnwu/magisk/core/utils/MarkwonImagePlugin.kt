package com.topjohnwu.magisk.core.utils

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Rect
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.Drawable
import android.graphics.drawable.PictureDrawable
import android.graphics.drawable.ShapeDrawable
import android.net.Uri
import android.text.Spanned
import android.text.style.DynamicDrawableSpan
import android.view.View
import android.widget.TextView
import androidx.annotation.WorkerThread
import androidx.core.view.marginLeft
import androidx.core.view.marginRight
import com.caverock.androidsvg.SVG
import com.caverock.androidsvg.SVGParseException
import com.topjohnwu.magisk.core.ResMgr
import com.topjohnwu.superuser.internal.WaitRunnable
import io.noties.markwon.*
import io.noties.markwon.image.*
import io.noties.markwon.image.data.DataUriSchemeHandler
import io.noties.markwon.image.network.OkHttpNetworkSchemeHandler
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.async
import kotlinx.coroutines.awaitAll
import kotlinx.coroutines.runBlocking
import okhttp3.OkHttpClient
import org.commonmark.node.Image
import timber.log.Timber
import java.io.InputStream

// Differences with Markwon stock ImagePlugin:
//
// We assume beforeSetText() will be run in a background thread, and in that method
// we download/decode all drawables before sending the spanned markdown CharSequence
// to the next stage. We also get our surrounding TextView width to properly
// resize our images.
//
// This is required for PrecomputedText to properly take the images into account
// when precomputing the metrics of TextView
//
// Basically, we want nothing to do with AsyncDrawable
class MarkwonImagePlugin(okHttp: OkHttpClient) : AbstractMarkwonPlugin() {

    override fun configureSpansFactory(builder: MarkwonSpansFactory.Builder) {
        builder.setFactory(Image::class.java, ImageSpanFactory())
    }

    @WorkerThread
    override fun beforeSetText(tv: TextView, markdown: Spanned) {
        if (markdown.isEmpty())
            return

        val spans = markdown.getSpans(0, markdown.length, ImageSpan::class.java)
        if (spans == null || spans.isEmpty())
            return

        // Download and create all drawables in parallel
        runBlocking(Dispatchers.IO) {
            // Download and decode all drawables in parallel
            val defers = spans.map { async { it.load() } }

            // Get TextView sizes beforehand to resize all images
            // We get its parent here as the TextView itself might be hidden
            val parent = tv.parent as View
            parent.post(WaitRunnable{
                // Make sure it is rendered
                val width = parent.width -
                        tv.paddingLeft - tv.paddingRight -
                        tv.marginLeft - tv.marginRight
                spans.forEach { it.canvasWidth = width }
            })

            // Make sure all is done before returning
            defers.awaitAll()
        }
    }

    private val schemeHandlers = HashMap<String, SchemeHandler>(3)
    private val mediaDecoders = HashMap<String, MediaDecoder>(0)
    private val defaultMediaDecoder = DefaultMediaDecoder.create()

    init {
        addSchemeHandler(DataUriSchemeHandler.create())
        addSchemeHandler(OkHttpNetworkSchemeHandler.create(okHttp))
        addMediaDecoder(SVGDecoder())
    }

    private fun addSchemeHandler(schemeHandler: SchemeHandler) {
        for (scheme in schemeHandler.supportedSchemes()) {
            schemeHandlers[scheme] = schemeHandler
        }
    }

    private fun addMediaDecoder(mediaDecoder: MediaDecoder) {
        for (type in mediaDecoder.supportedTypes()) {
            mediaDecoders[type] = mediaDecoder
        }
    }

    // Modified from AsyncDrawableLoaderImpl.execute(asyncDrawable)
    fun loadDrawable(destination: String): Drawable? {
        val uri = Uri.parse(destination)
        var drawable: Drawable? = null

        try {
            val scheme = uri.scheme
            check(scheme != null && scheme.isNotEmpty()) {
                "No scheme is found: $destination"
            }

            // obtain scheme handler
            val schemeHandler = schemeHandlers[scheme]
                ?: throw IllegalStateException("No scheme-handler is found: $destination")

            // handle scheme
            val imageItem = schemeHandler.handle(destination, uri)

            // if resulting imageItem needs further decoding -> proceed
            drawable = if (imageItem.hasDecodingNeeded()) {
                val withDecodingNeeded = imageItem.asWithDecodingNeeded
                val mediaDecoder = mediaDecoders[withDecodingNeeded.contentType()]
                    ?: defaultMediaDecoder
                mediaDecoder.decode(
                    withDecodingNeeded.contentType(),
                    withDecodingNeeded.inputStream()
                )
            } else {
                imageItem.asWithResult.result()
            }
        } catch (t: Throwable) {
            Timber.e(t, "Error loading image: $destination")
        }

        // apply intrinsic bounds (but only if they are empty)
        if (drawable != null && drawable.bounds.isEmpty)
            DrawableUtils.applyIntrinsicBounds(drawable)

        return drawable
    }

    inner class ImageSpanFactory : SpanFactory {
        override fun getSpans(configuration: MarkwonConfiguration, props: RenderProps): Any? {
            val dest = ImageProps.DESTINATION.require(props)
            val size = ImageProps.IMAGE_SIZE.get(props)
            return ImageSpan(dest, size)
        }
    }

    inner class ImageSpan(
        private val dest: String,
        private val size: ImageSize?
    ) : DynamicDrawableSpan(ALIGN_BOTTOM) {

        var canvasWidth = 0
        var measured = false
        lateinit var draw: Drawable

        fun load() {
            draw = loadDrawable(dest) ?: ShapeDrawable()
        }

        override fun getDrawable() = draw

        private fun defaultBounds(): Rect {
            val bounds: Rect = draw.bounds
            if (!bounds.isEmpty) {
                return bounds
            }
            val intrinsicBounds = DrawableUtils.intrinsicBounds(draw)
            if (!intrinsicBounds.isEmpty) {
                return intrinsicBounds
            }
            return Rect(0, 0, 1, 1)
        }

        private fun measure(paint: Paint) {
            if (measured)
                return
            if (canvasWidth == 0)
                return
            measured = true
            val bound = SizeResolver.resolveImageSize(size, defaultBounds(), canvasWidth, paint.textSize)
            draw.bounds = bound
        }

        override fun getSize(
            paint: Paint, text: CharSequence?, start: Int, end: Int, fm: Paint.FontMetricsInt?
        ): Int {
            measure(paint)
            return super.getSize(paint, text, start, end, fm)
        }
    }

    object SizeResolver : ImageSizeResolverDef() {
        // Expose protected API
        public override fun resolveImageSize(
            imageSize: ImageSize?,
            imageBounds: Rect,
            canvasWidth: Int,
            textSize: Float
        ): Rect {
            return super.resolveImageSize(imageSize, imageBounds, canvasWidth, textSize)
        }
    }

    class SVGDecoder: MediaDecoder() {
        override fun supportedTypes() = listOf("image/svg+xml")

        override fun decode(contentType: String?, inputStream: InputStream): Drawable {
            val svg: SVG
            svg = try {
                SVG.getFromInputStream(inputStream)
            } catch (e: SVGParseException) {
                throw IllegalStateException("Exception decoding SVG", e)
            }

            val w = svg.documentWidth
            val h = svg.documentHeight

            if (w == 0f || h == 0f) {
                val picture = svg.renderToPicture()
                return PictureDrawable(picture)
            }

            val density: Float = ResMgr.resource.displayMetrics.density

            val width = (w * density + .5f).toInt()
            val height = (h * density + .5f).toInt()

            val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
            val canvas = Canvas(bitmap)
            canvas.scale(density, density)
            svg.renderToCanvas(canvas)

            return BitmapDrawable(ResMgr.resource, bitmap)
        }

    }
}
