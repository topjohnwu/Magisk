# Magisk

### How to build Magisk

1. Download and install NDK
2. Add the NDK directory into `PATH`
To check if the `PATH` environment variable is set correctly, try calling `which ndk-build` (`where ndk-build` on Windows) and see if it shows the NDK directory you've just configured
3. Unix-like users (e.g. Linux & MacOS) please execute `build.sh` through shell  
Windows users please execute `build.cmd` through cmd
4. The scripts will show you further instructions and details

### License

Magisk, including all subprojects (git submodule) is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

### Credits

**MagiskSU** (`jni/su`)
* Copyright 2016-2017, John Wu (@topjohnwu)
* Copyright 2015, Pierre-Hugues Husson (phh@phh.me)
* Copyright 2013, Koushik Dutta (@koush)
* Copyright 2010, Adam Shanks (@ChainsDD)
* Copyright 2008, Zinx Verituse (@zinxv)

**MagiskPolicy** (`jni/magiskpolicy`)
* Copyright 2016-2017, John Wu (@topjohnwu)
* Copyright 2015, Pierre-Hugues Husson (phh@phh.me)
* Copyright 2015, Joshua Brindle (@joshua_brindle)

**MagiskHide** (`jni/magiskhide`)
* Copyright 2016-2017, John Wu (@topjohnwu)
* Copyright 2016, Pierre-Hugues Husson (phh@phh.me) (original hidesu)

**resetprop** (`jni/resetprop`)
 * Copyright 2016-2017 John Wu (@topjohnwu)
 * Copyright 2016 nkk71 (nkk71x@gmail.com)
 
**SELinux** (`jni/selinux`)
* Makefile for NDK: Copyright 2016-2017, John Wu (@topjohnwu)
* It is maintained by many developers in SELinux project, copyright belongs to them

**ndk-compression** (`jni/ndk-compression`)
* Makefile for NDK: Copyright 2017, John Wu (@topjohnwu)
* Each library has its own copyright message in each directories

**Anything Not Mentioned**
* Copyright 2016-2017, John Wu (@topjohnwu)
