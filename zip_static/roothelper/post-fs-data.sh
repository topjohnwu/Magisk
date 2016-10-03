#!/system/bin/sh

cd /magisk/00roothelper

if [ -f "launch_daemonsu.sh" ]; then
  # SuperSU mode
  rm -rf /magisk/supersu /magisk/zzsupersu
  mkdir -p /magisk/zzsupersu
  cp supersu.sh /magisk/zzsupersu/post-fs-data.sh
  cp supersu.prop /magisk/zzsupersu/module.prop
  cp launch_daemonsu.sh /magisk/zzsupersu/launch_daemonsu.sh
  chmod 755 /magisk/zzsupersu /magisk/zzsupersu/*
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
