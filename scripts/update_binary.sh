# EX_ARM, EX_X86, BB_ARM, and BB_X86 should be generated in build.py
TMPDIR=/dev/tmp; INSTALLER=$TMPDIR/install; BBDIR=$TMPDIR/bin
EXBIN=$BBDIR/b64xz; BBBIN=$BBDIR/busybox
rm -rf $TMPDIR 2>/dev/null; mkdir -p $BBDIR
touch $EXBIN $BBBIN; chmod 755 $EXBIN $BBBIN
echo -ne $EX_ARM > $EXBIN
if $EXBIN --test 2>/dev/null; then
  echo $BB_ARM | $EXBIN > $BBBIN
else
  echo -ne $EX_x86 > $EXBIN
  echo $BB_x86 | $EXBIN > $BBBIN
fi
$BBBIN --install -s $TMPDIR/bin
export PATH=$BBDIR:$PATH
mkdir -p $INSTALLER
unzip -o "$3" -d $INSTALLER
exec sh $INSTALLER/META-INF/com/google/android/updater-script $@
