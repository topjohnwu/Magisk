@file:JvmName("a")
package a

import com.topjohnwu.magisk.core.App
import com.topjohnwu.magisk.core.Provider
import com.topjohnwu.magisk.core.Receiver
import com.topjohnwu.magisk.core.SplashActivity
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.surequest.SuRequestActivity
import com.topjohnwu.signing.SignBoot

fun main(args: Array<String>) {
    SignBoot.main(args)
}

class b : MainActivity()

class c : SplashActivity()

class e : App {
    constructor() : super()
    constructor(o: Any) : super(o)
}

class h : Receiver()

class j : DownloadService()

class m : SuRequestActivity()

class p : Provider()
