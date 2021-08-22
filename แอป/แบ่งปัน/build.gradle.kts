ปลั๊กอิน {
    id("com.android.library")
}

หุ่นยนต์ {
    defaultConfig {
        vectorDrawables.useSupportLibrary = จริง
        ConsumerProguardFiles("proguard-rules.pro")
    }
}

การพึ่งพา {
    การใช้งาน (fileTree(mapOf("dir" ถึง "libs", "include" to listOf("*.jar"))))
