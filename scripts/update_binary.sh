#!/sbin/sh

TMPDIR=/dev/tmp
rm -rf $TMPDIR
mkdir -p $TMPDIR/bb 2>/dev/null

export BBBIN=$TMPDIR/busybox
unzip -o "$3" -d $TMPDIR/bb >&2
chmod -R 755 $TMPDIR/bb/lib
for arch in "x86_64" "x86" "arm64-v8a" "armeabi-v7a"; do
  libpath="$TMPDIR/bb/lib/$arch/libbusybox.so"
  if [ -x $libpath ] && $libpath >/dev/null 2>&1; then
    mv -f $libpath $BBBIN
    break
  fi
done
$BBBIN rm -rf $TMPDIR/bb

export INSTALLER=$TMPDIR/install
$BBBIN mkdir -p $INSTALLER
$BBBIN unzip -o "$3" "assets/*" "lib/*" "META-INF/com/google/*" -x "lib/*/libbusybox.so" -d $INSTALLER >&2
export ASH_STANDALONE=1
if echo "$3" | $BBBIN grep -q "uninstall"; then
  exec $BBBIN sh "$INSTALLER/assets/uninstaller.sh" "$@"
else
  exec $BBBIN sh "$INSTALLER/META-INF/com/google/android/updater-script" "$@"
fi
