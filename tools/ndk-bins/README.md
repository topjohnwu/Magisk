## Prebuilt Static Libraries

These binaries are extracted straight from [Google NDK r10e](https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip).
It contains the Bionic libc from Lollipop.

Over the years, Bionic has slowly become more and more gigantic as devices became more powerful and Google kept adding new features.
These are the oldest, which also means smallest, Bionic libc Google had ever offered.
We prefer to use these static libs because they yield significantly smaller static executables.
