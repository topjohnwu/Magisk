# EX_ARM, EX_X86, BB_ARM, and BB_X86 should be generated in build.py
dirname_wrap() {
  case "$1" in
    */*) dir=${1%/*}; [ -z $dir ] && echo "/" || echo $dir ;;
    *) echo "." ;;
  esac
}
[ "$1" = "indep" ] && INDEP=true || INDEP=false
$INDEP && TMPDIR="`dirname_wrap "${BASH_SOURCE:-$0}"`" || TMPDIR=/dev/tmp
INSTALLER=$TMPDIR/install; BBDIR=$TMPDIR/bin
EXBIN=$BBDIR/b64xz; BBBIN=$BBDIR/busybox
$INDEP || rm -rf $TMPDIR 2>/dev/null;
mkdir -p $BBDIR 2>/dev/null
touch $EXBIN $BBBIN; chmod 755 $EXBIN $BBBIN
echo -ne $EX_ARM > $EXBIN
if $EXBIN --test 2>/dev/null; then
  echo $BB_ARM | $EXBIN > $BBBIN
else
  echo -ne $EX_X86 > $EXBIN
  echo $BB_X86 | $EXBIN > $BBBIN
fi
$BBBIN --install -s $TMPDIR/bin
export PATH=$BBDIR:$PATH
if $INDEP; then
  shift
  exec sh "$@"
else
  mkdir -p $INSTALLER
  unzip -o "$3" -d $INSTALLER
  exec sh $INSTALLER/META-INF/com/google/android/updater-script $@
fi
