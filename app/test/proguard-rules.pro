# Keep all test dependencies
-keep class org.junit.** { *; }
-keep class androidx.test.** { *; }

# Make sure the classloader is kept
-keepclassmembers class com.topjohnwu.magisk.test.TestClassLoader {
    <init>();
}

# Excessive obfuscation
-repackageclasses 'deps'
-allowaccessmodification
