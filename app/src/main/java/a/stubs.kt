@file:JvmName("a")
package a

import com.topjohnwu.magisk.ProcessPhoenix
import com.topjohnwu.magisk.core.App
import com.topjohnwu.magisk.core.GeneralReceiver
import com.topjohnwu.magisk.core.SplashActivity
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.signing.BootSigner

fun main(args: Array<String>) {
    BootSigner.main(args)
}

class b : MainActivity()

class c : SplashActivity()

class e : App {
    constructor() : super()
    constructor(o: Any) : super(o)
}

class h : GeneralReceiver()

class j : DownloadService()

class m : SuRequestActivity()

class r : ProcessPhoenix()
