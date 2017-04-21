#!/bin/bash

usage() {
  echo "$ME all <version name> <version code>"
  echo -e "\tBuild binaries, zip, and sign Magisk"
  echo -e "\tThis is equlivant to first <build>, then <zip>"
  echo "$ME clean"
  echo -e "\tCleanup compiled / generated files"
  echo "$ME build <verison name> <version code>"
  echo -e "\tBuild the binaries with ndk"
  echo "$ME debug <verison name> <version code>"
  echo -e "\tBuild the binaries with the debug flag on\n\tCall <zip> afterwards if you want to flash on device"
  echo "$ME zip <version name>"
  echo -e "\tZip and sign Magisk"
  echo "$ME uninstaller"
  echo -e "\tZip and sign the uninstaller"
  exit 1
}

cleanup() {
  echo "************************"
  echo "* Cleaning up"
  echo "************************"
  ndk-build clean 2>/dev/null
  rm -rfv zip_static/arm
  rm -rfv zip_static/arm64
  rm -rfv zip_static/x86
  rm -rfv zip_static/x64
  rm -rfv zip_static/META-INF/com/google/android/update-binary
  rm -rfv zip_static/common/*.sh
  rm -rfv zip_static/common/*.rc
  rm -rfv uninstaller/common
  rm -rfv uninstaller/arm
  rm -rfv uninstaller/arm64
  rm -rfv uninstaller/x86
  rm -rfv uninstaller/x64
}

mkcp() {
  [ ! -d "$2" ] && mkdir -p "$2"
  cp -afv $1 $2
}

error() {
  echo -e "\n! $1\n"
  exit 1
}

build_bin() {
  [ -z "$1" -o -z "$2" ] && echo -e "! Missing version info\n" && usage
  BUILD="Build"
  [ -z "DEBUG" ] || BUILD="Debug"
  echo "************************"
  echo "* $BUILD: VERSION=$1 CODE=$2"
  echo "************************"
  [ -z `which ndk-build` ] && error "Please add ndk-build to PATH!"
  ndk-build APP_CFLAGS="-DMAGISK_VERSION=$1 -DMAGISK_VER_CODE=$2 $DEBUG" -j${CPUNUM} || error "Magisk binary tools build failed...."
  echo "************************"
  echo "* Copying binaries"
  echo "************************"
  mkcp libs/armeabi-v7a/magisk zip_static/arm
  mkcp libs/armeabi-v7a/magiskboot zip_static/arm
  mkcp libs/arm64-v8a/magisk zip_static/arm64
  mkcp libs/arm64-v8a/magiskboot zip_static/arm64
  mkcp libs/x86/magisk zip_static/x86
  mkcp libs/x86/magiskboot zip_static/x86
  mkcp libs/x86_64/magisk zip_static/x64
  mkcp libs/x86_64/magiskboot zip_static/x64
  mkcp libs/armeabi-v7a/magiskboot uninstaller/arm
  mkcp libs/arm64-v8a/magiskboot uninstaller/arm64
  mkcp libs/x86/magiskboot uninstaller/x86
  mkcp libs/x86_64/magiskboot uninstaller/x64
}

zip_package() {
  [ -z "$1" ] && echo -e "! Missing version info\n" && usage
  [ ! -f "zip_static/arm/magiskboot" ] && error "Missing binaries!! Please run '$ME build' before zipping"
  echo "************************"
  echo "* Adding version info"
  echo "************************"
  sed "s/MAGISK_VERSION_STUB/Magisk v$1 Installer/g" scripts/flash_script.sh > zip_static/META-INF/com/google/android/update-binary
  # sed "s/MAGISK_VERSION_STUB/setprop magisk.version \"$1\"/g" scripts/magic_mask.sh > zip_static/common/magic_mask.sh
  echo "************************"
  echo "* Copying files"
  echo "************************"
  cp -afv scripts/custom_ramdisk_patch.sh zip_static/common/custom_ramdisk_patch.sh
  cp -afv scripts/init.magisk.rc zip_static/common/init.magisk.rc
  echo "************************"
  echo "* Zipping Magisk v$1"
  echo "************************"
  cd zip_static
  find . -type f -exec chmod 644 {} \;
  find . -type d -exec chmod 755 {} \;
  rm -rf "../Magisk-v$1.zip"
  zip "../Magisk-v$1.zip" -r .
  cd ../
  sign_zip "Magisk-v$1.zip"
}

zip_uninstaller() {
  [ ! -f "uninstaller/arm/magiskboot" ] && error "Missing binaries!! Please run '$0 build' before zipping"
  echo "************************"
  echo "* Copying files"
  echo "************************"
  mkcp scripts/magisk_uninstaller.sh uninstaller/common
  echo "************************"
  echo "* Zipping uninstaller"
  echo "************************"
  cd uninstaller
  find . -type f -exec chmod 644 {} \;
  find . -type d -exec chmod 755 {} \;
  TIMESTAMP=`date "+%Y%m%d"`
  rm -rf "../Magisk-uninstaller-$TIMESTAMP.zip"
  zip "../Magisk-uninstaller-$TIMESTAMP.zip" -r .
  cd ../
  sign_zip "Magisk-uninstaller-$TIMESTAMP.zip"
}

sign_zip() {
  if [ ! -f "ziptools/zipadjust" ]; then
    echo "************************"
    echo "* Compiling ZipAdjust"
    echo "************************"
    gcc -o ziptools/zipadjust ziptools/src/*.c -lz || error "ZipAdjust Build failed...."
    chmod 755 ziptools/zipadjust
  fi
  echo "************************"
  echo "* First sign $1"
  echo "************************"
  java -jar "ziptools/signapk.jar" "ziptools/test.certificate.x509.pem" "ziptools/test.key.pk8" "$1" "${1%.*}-firstsign.zip"
  echo "************************"
  echo "* Adjusting $1"
  echo "************************"
  ziptools/zipadjust "${1%.*}-firstsign.zip" "${1%.*}-adjusted.zip"
  echo "************************"
  echo "* Final sign $1"
  echo "************************"
  java -jar "ziptools/minsignapk.jar" "ziptools/test.certificate.x509.pem" "ziptools/test.key.pk8" "${1%.*}-adjusted.zip" "${1%.*}-signed.zip"

  mv "${1%.*}-signed.zip" "$1"
  rm "${1%.*}-adjusted.zip" "${1%.*}-firstsign.zip"
}

DIR="$(cd "$(dirname "$0")"; pwd)"
DEBUG=
CPUNUM=`getconf _NPROCESSORS_ONLN`
ME=$0

case $1 in
  "all" )
    build_bin $2 $3
    zip_package $2
    ;;
  "clean" )
    cleanup
    ;;
  "build" )
    build_bin $2 $3
    ;;
  "debug" )
    DEBUG="-DDEBUG"
    build_bin $2 $3
    ;;
  "zip" )
    [ -z "$2" ] && echo -e "! Missing version info\n" && usage
    zip_package $2
    ;;
  "uninstaller" )
    zip_uninstaller
    ;;
  * )
    usage
    ;;
esac
