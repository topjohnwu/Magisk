#!/sbin/sh
X86_CNT=__X86_CNT__
extract_bb() {
  touch "$BBBIN"
  chmod 755 "$BBBIN"
  dd if="$0" of="$BBBIN" bs=1024 skip=1 count=$X86_CNT
  "$BBBIN" >/dev/null 2>&1 || dd if="$0" of="$BBBIN" bs=1024 skip=$(($X86_CNT + 1))
}
setup_bb() {
  mkdir -p $TMPDIR 2>/dev/null
  BBBIN=$TMPDIR/busybox
  extract_bb
}
export BBBIN
case "$1" in
  "extract"|"-x")
    [ -z "$2" ] && BBBIN=./busybox || BBBIN="$2"
    extract_bb
    ;;
  "sh")
    TMPDIR=.
    setup_bb
    shift
    exec ./busybox sh -o standalone "$@"
    ;;
  *)
    TMPDIR=/dev/tmp
    rm -rf $TMPDIR 2>/dev/null
    setup_bb
    export INSTALLER=$TMPDIR/install
    $BBBIN mkdir -p $INSTALLER
    $BBBIN unzip -o "$3" -d $INSTALLER >&2
    exec $BBBIN sh -o standalone $INSTALLER/META-INF/com/google/android/updater-script "$@"
    ;;
esac
exit
