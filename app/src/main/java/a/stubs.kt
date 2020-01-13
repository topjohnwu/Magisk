package a

import android.content.Context
import androidx.work.Worker
import androidx.work.WorkerParameters
import com.topjohnwu.magisk.core.App
import com.topjohnwu.magisk.core.GeneralReceiver
import com.topjohnwu.magisk.core.SplashActivity
import com.topjohnwu.magisk.core.base.BaseWorkerWrapper
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.legacy.flash.FlashActivity
import com.topjohnwu.magisk.legacy.surequest.SuRequestActivity
import com.topjohnwu.magisk.ui.MainActivity
import java.lang.reflect.ParameterizedType

class b : MainActivity()

class c : SplashActivity()

class e : App {
    constructor() : super()
    constructor(o: Any) : super(o)
}

class f : FlashActivity()

class h : GeneralReceiver()

class j : DownloadService()

class m : SuRequestActivity()

/**
 * Wrapper class to workaround Proguard rule :
 * -keep class * extends Worker
 * */
abstract class w<T : BaseWorkerWrapper>(
    context: Context,
    workerParams: WorkerParameters
) : Worker(context, workerParams) {

    private var base: T? = null

    override fun doWork() = base?.doWork() ?: Result.failure()

    override fun onStopped() = base?.onStopped() ?: Unit

    init {
        try {
            base = ((javaClass.genericSuperclass as ParameterizedType)
                .actualTypeArguments[0] as Class<T>).newInstance()
            base?.attachWorker(this)
        } catch (e : java.lang.Exception) {}
    }
}
