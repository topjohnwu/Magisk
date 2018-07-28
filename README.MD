# Magisk

## Building Environment Requirements

1. Python 3.5+: run `build.py` script
2. Java Development Kit (JDK) 8: Compile Magisk Manager and sign zips
3. Latest Android SDK: set `ANDROID_HOME` environment variable to the path to Android SDK
4. Android NDK: Install NDK along with SDK (`$ANDROID_HOME/ndk-bundle`), or optionally specify a custom path `ANDROID_NDK_HOME`
5. (Windows Only) Python package Colorama: Install with `pip install colorama`, used for ANSI color codes

## Building Notes and Instructions
1. Building is tested on macOS, Ubuntu, and Windows 10 using the latest stable NDK and NDK r10e. Officially released binaries were built with NDK r10e.
2. Set configurations in `config.prop`. A sample file `config.prop.sample` is provided as an example.
3. Run `build.py` with argument `-h` to see the built-in help message. The `-h` option also works for each supported actions, e.g. `./build.py binary -h`
4. By default, `build.py` build binaries and Magisk Manager in debug mode. If you want to build Magisk Manager in release mode (via the `--release` flag), you need a Java Keystore file `release-key.jks` to sign Magisk Manager's APK. For more information, check out [Google's Official Documentation](https://developer.android.com/studio/publish/app-signing.html#signing-manually).

## Documentation
[Link to Documentation](docs/README.MD)

## License

```
Magisk, including all git submodules are free software:
you can redistribute it and/or modify it under the terms of the 
GNU General Public License as published by the Free Software Foundation, 
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
```
