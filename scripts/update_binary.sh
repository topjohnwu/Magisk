# BB_ARM and BB_X86 should be generated in build.py
TMPDIR=/dev/tmp
INSTALLER=$TMPDIR/install
BBDIR=$TMPDIR/bin
BBBIN=$BBDIR/busybox
rm -rf $TMPDIR 2>/dev/null; mkdir -p $BBDIR; touch $BBBIN; chmod 755 $BBBIN
echo $BB_ARM | base64 -d | gzip -d > $BBBIN
if ! $BBBIN --install -s $TMPDIR/bin >/dev/null 2>&1; then
  echo $BB_X86 | base64 -d | gzip -d > $BBBIN
  $BBBIN --install -s $TMPDIR/bin >/dev/null 2>&1 || exit 1
fi
export PATH=$BBDIR:$PATH
mkdir -p $INSTALLER
unzip -o "$3" -d $INSTALLER
exec sh $INSTALLER/META-INF/com/google/android/updater-script $@
