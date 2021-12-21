#!/usr/bin/env bash
set -x

BOT_DIR=/b
BOT_NAME=$1
BOT_PASS=$2

mkdir -p $BOT_DIR

#curl "https://repo.stackdriver.com/stack-install.sh" | bash -s -- --write-gcm

apt-get update -y
apt-get upgrade -y

# FIXME(EricWF): Remove this hack. It's only in place to temporarily fix linking libclang_rt from the
# debian packages.
# WARNING: If you're not a buildbot, DO NOT RUN!
apt-get install lld-8
rm /usr/bin/ld
ln -s /usr/bin/lld-8 /usr/bin/ld

systemctl set-property buildslave.service TasksMax=100000

buildslave stop $BOT_DIR

chown buildbot:buildbot $BOT_DIR

echo "Connecting as $BOT_NAME"
buildslave create-slave --allow-shutdown=signal $BOT_DIR lab.llvm.org:9990 $BOT_NAME $BOT_PASS

echo "Eric Fiselier <ericwf@google.com>" > $BOT_DIR/info/admin

{
  uname -a | head -n1
  cmake --version | head -n1
  g++ --version | head -n1
  ld --version | head -n1
  date
  lscpu
} > $BOT_DIR/info/host

echo "SLAVE_RUNNER=/usr/bin/buildslave
SLAVE_ENABLED[1]=\"1\"
SLAVE_NAME[1]=\"buildslave1\"
SLAVE_USER[1]=\"buildbot\"
SLAVE_BASEDIR[1]=\"$BOT_DIR\"
SLAVE_OPTIONS[1]=\"\"
SLAVE_PREFIXCMD[1]=\"\"" > /etc/default/buildslave

chown -R buildbot:buildbot $BOT_DIR
systemctl daemon-reload
service buildslave restart

sleep 30
cat $BOT_DIR/twistd.log
grep "slave is ready" $BOT_DIR/twistd.log || shutdown now

# GCE can restart instance after 24h in the middle of the build.
# Gracefully restart before that happen.
sleep 72000
while pkill -SIGHUP buildslave; do sleep 5; done;
shutdown now