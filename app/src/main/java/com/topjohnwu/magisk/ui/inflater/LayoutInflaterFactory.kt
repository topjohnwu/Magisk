package com.topjohnwu.magisk.ui.inflater

import android.content.Context
import android.os.Build
import android.util.AttributeSet
import android.view.InflateException
import android.view.LayoutInflater
import android.view.View
import androidx.appcompat.app.AppCompatDelegate
import androidx.collection.SimpleArrayMap
import java.lang.reflect.Constructor

open class LayoutInflaterFactory(private val delegate: AppCompatDelegate) : LayoutInflater.Factory2 {

    override fun onCreateView(name: String, context: Context, attrs: AttributeSet): View? {
        return onCreateView(null, name, context, attrs)
    }

    override fun onCreateView(parent: View?, name: String, context: Context, attrs: AttributeSet): View? {
        val view = delegate.createView(parent, name, context, attrs)
            ?: LayoutInflaterFactoryDefaultImpl.createViewFromTag(context, name, attrs)
        onViewCreated(view, parent, name, context, attrs)
        return view
    }

    open fun onViewCreated(view: View?, parent: View?, name: String, context: Context, attrs: AttributeSet) {
        if (view == null) return

        WindowInsetsHelper.attach(view, attrs)
    }
}

private object LayoutInflaterFactoryDefaultImpl {

    private val constructorSignature = arrayOf(
        Context::class.java, AttributeSet::class.java)

    private val classPrefixList = arrayOf(
        "android.widget.",
        "android.view.",
        "android.webkit."
    )

    private val constructorMap = SimpleArrayMap<String, Constructor<out View?>>()

    fun createViewFromTag(context: Context, name: String, attrs: AttributeSet): View? {
        var name = name
        if (name == "view") {
            name = attrs.getAttributeValue(null, "class")
        }
        return try {
            if (-1 == name.indexOf('.')) {
                for (prefix in classPrefixList) {
                    val view: View? = createViewByPrefix(context, name, attrs, prefix)
                    if (view != null) {
                        return view
                    }
                }
                null
            } else {
                createViewByPrefix(context, name, attrs, null)
            }
        } catch (e: Exception) {
            null
        }
    }

    @Throws(ClassNotFoundException::class, InflateException::class)
    private fun createViewByPrefix(context: Context, name: String, attrs: AttributeSet, prefix: String?): View? {
        var constructor = constructorMap[name]
        return try {
            if (constructor == null) { // Class not found in the cache, see if it's real, and try to add it
                val clazz = Class.forName(
                    if (prefix != null) prefix + name else name,
                    false,
                    context.classLoader).asSubclass(View::class.java)
                constructor = clazz.getConstructor(*constructorSignature)
                constructorMap.put(name, constructor)
            }
            constructor!!.isAccessible = true
            constructor.newInstance(context, attrs)
        } catch (e: Exception) {
            null
        }
    }
}
