#!/sbin/sh
X86_CNT=__X86_CNT__
extract_bb() {
  touch "$BBBIN"
  chmod 755 "$BBBIN"
  dd if="$0" of="$BBBIN" bs=1024 skip=$(($X86_CNT + 1))
  "$BBBIN" >/dev/null || dd if="$0" of="$BBBIN" bs=1024 skip=1 count=$X86_CNT
}
setup_bb() {
  export BBDIR=$TMPDIR/bin
  BBBIN=$BBDIR/busybox
  mkdir -p $BBDIR 2>/dev/null
  extract_bb
  $BBBIN --install -s $BBDIR
  export PATH=$BBDIR:$PATH
}
case "$1" in
  "extract"|"-x")
    [ -z "$2" ] && BBBIN=./busybox || BBBIN="$2"
    extract_bb
    ;;
  "indep"|"sh")
    TMPDIR=.
    setup_bb
    shift
    exec /system/bin/sh "$@"
    ;;
  *)
    export TMPDIR=/dev/tmp
    rm -rf $TMPDIR 2>/dev/null
    setup_bb
    export INSTALLER=$TMPDIR/install
    mkdir -p $INSTALLER
    unzip -o "$3" -d $INSTALLER >&2
    exec sh $INSTALLER/META-INF/com/google/android/updater-script "$@"
    ;;
esac
exit
