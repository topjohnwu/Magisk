# Keep all test dependencies
-keep class org.junit.** { *; }
-keep class androidx.test.** { *; }

# Make sure the classloader constructor is kept
-keepclassmembers class com.topjohnwu.magisk.test.TestClassLoader { <init>(); }

# Repackage dependencies
-repackageclasses 'deps'
-allowaccessmodification

# Keep attributes for stacktrace
-keepattributes *
