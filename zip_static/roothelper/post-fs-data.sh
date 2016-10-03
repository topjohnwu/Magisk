#!/system/bin/sh

cd /magisk/00roothelper

if [ -f "launch_daemonsu.sh" ]; then
  # SuperSU mode
  rm -rf /magisk/supersu /magisk/99supersu
  mkdir -p /magisk/99supersu
  cp supersu.sh /magisk/99supersu/post-fs-data.sh
  cp supersu.prop /magisk/99supersu/module.prop
  cp launch_daemonsu.sh /magisk/99supersu/launch_daemonsu.sh
  chmod 755 /magisk/99supersu /magisk/99supersu/*
else
  # phh mode
  if [ -f "/magisk/phh/su" ]; then
    # Old version detected
    cp /magisk/phh/su su
    rm -rf /magisk/phh
    mkdir -p /magisk/phh/bin
    mkdir -p /magisk/phh/su.d
    cp su /magisk/phh/bin/su
    cp /data/magisk/sepolicy-inject /magisk/phh/bin/sepolicy-inject
    cp phh.sh /magisk/phh/post-fs-data.sh
    cp phh.prop /magisk/phh/module.prop
    chmod 755 /magisk/phh /magisk/phh/* /magisk/phh/bin/*
  fi
fi

rm -rf /magisk/00roothelper
