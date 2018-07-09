# EX_ARM, EX_X86, BB_ARM, and BB_X86 should be generated in build.py
extract_bb() {
  EXBIN=$BBDIR/b64xz; BBBIN=$BBDIR/busybox
  touch $EXBIN; touch $BBBIN; chmod 755 $EXBIN $BBBIN
  echo -ne $EX_ARM > $EXBIN
  if $EXBIN --test 2>/dev/null; then
    echo $BB_ARM | $EXBIN > $BBBIN
  else
    echo -ne $EX_X86 > $EXBIN
    echo $BB_X86 | $EXBIN > $BBBIN
  fi
  rm $EXBIN
}
setup_bb() {
  BBDIR=$TMPDIR/bin; mkdir -p $BBDIR 2>/dev/null
  extract_bb
  $BBBIN --install -s $BBDIR
  export PATH=$BBDIR:$PATH
}
case "$1" in
  "extract")
    [ -z "$2" ] && BBDIR=. || BBDIR="$2"
    extract_bb
    ;;
  "indep")
    TMPDIR=.; setup_bb; shift
    exec /system/bin/sh "$@"
    ;;
  *)
    export TMPDIR=/dev/tmp; rm -rf $TMPDIR 2>/dev/null; setup_bb
    export INSTALLER=$TMPDIR/install; mkdir -p $INSTALLER; unzip -o "$3" -d $INSTALLER >&2
    exec sh $INSTALLER/META-INF/com/google/android/updater-script "$@"
    ;;
esac
