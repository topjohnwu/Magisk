#Extracted from global_macros
rw_socket_perms="ioctl read getattr write setattr lock append bind connect getopt setopt shutdown"
create_socket_perms="create $rw_socket_perms"
rw_stream_socket_perms="$rw_socket_perms listen accept"
create_stream_socket_perms="create $rw_stream_socket_perms"

# bootimg.sh

#allow <list of scontext> <list of tcontext> <class> <list of perm>
allow() {
  [ -z "$1" -o -z "$2" -o -z "$3" ] && false
  for s in $1;do
    for t in $2;do
      for c in $3;do
        echo "allow ($s) ($t) ($c) ($4)"
        if [ "$4" = "*" ]; then
          $BINDIR/sepolicy-inject -s $s -t $t -c $c -P sepolicy >/dev/null 2>&1
        else
          $BINDIR/sepolicy-inject -s $s -t $t -c $3 -p $(echo $4|tr ' ' ',') -P sepolicy 2>/dev/null 2>&1
        fi
      done
    done
  done
}

noaudit() {
  for s in $1;do
    for t in $2;do
      for p in $4;do
        $BINDIR/sepolicy-inject -s $s -t $t -c $3 -p $p -P sepolicy 2>/dev/null 2>&1
      done
    done
  done
}

# su-communication

#allowSuClient <scontext>
allowSuClient() {
  #All domain-s already have read access to rootfs
  allow $1 rootfs file "execute_no_trans execute" #TODO: Why do I need execute?!? (on MTK 5.1, kernel 3.10)
  allow $1 su_daemon unix_stream_socket "connectto getopt"

  allow $1 su_device dir "search read"
  allow $1 su_device sock_file "read write"
  allow su_daemon $1 "fd" "use"

  allow su_daemon $1 fifo_file "read write getattr ioctl"

  #Read /proc/callerpid/cmdline in from_init, drop?
  #Requiring sys_ptrace sucks
  allow su_daemon "$1" "dir" "search"
  allow su_daemon "$1" "file" "read open"
  allow su_daemon "$1" "lnk_file" "read"
  allow su_daemon su_daemon "capability" "sys_ptrace"
}

suDaemonTo() {
  allow su_daemon $1 "process" "transition"
  noaudit su_daemon $1 "process" "siginh rlimitinh noatsecure"
}

suDaemonRights() {
  allow su_daemon rootfs file "entrypoint"

  allow su_daemon su_daemon "dir" "search read"
  allow su_daemon su_daemon "file" "read write open"
  allow su_daemon su_daemon "lnk_file" "read"
  allow su_daemon su_daemon "unix_dgram_socket" "create connect write"
  allow su_daemon su_daemon "unix_stream_socket" "$create_stream_socket_perms"

  allow su_daemon devpts chr_file "read write open getattr"
  #untrusted_app_devpts not in Android 4.4
  allow su_daemon untrusted_app_devpts chr_file "read write open getattr" || true

  allow su_daemon su_daemon "capability" "setuid setgid"

  #Access to /data/data/me.phh.superuser/xxx
  allow su_daemon app_data_file "dir" "getattr search write add_name"
  allow su_daemon app_data_file "file" "getattr read open lock"

  #FIXME: This shouldn't exist
  #dac_override can be fixed by having pts_slave's fd forwarded over socket
  #Instead of forwarding the name
  allow su_daemon su_daemon "capability" "dac_override"

  allow su_daemon su_daemon "process" "fork sigchld"

  #toolbox needed for log
  allow su_daemon toolbox_exec "file" "execute read open execute_no_trans" || true

  #Create /dev/me.phh.superuser. Could be done by init
  allow su_daemon device "dir" "write add_name"
  allow su_daemon su_device "dir" "create setattr remove_name add_name"
  allow su_daemon su_device "sock_file" "create unlink"

  #Allow su daemon to start su apk
  allow su_daemon zygote_exec "file" "execute read open execute_no_trans"
  allow su_daemon zygote_exec "lnk_file" "read getattr"

  #Send request to APK
  allow su_daemon su_device dir "search write add_name"

  #Allow su_daemon to switch to su or su_sensitive
  allow su_daemon su_daemon "process" "setexec"

  #Allow su_daemon to execute a shell (every commands are supposed to go through a shell)
  allow su_daemon shell_exec file "execute read open"

  allow su_daemon su_daemon "capability" "chown"

  suDaemonTo su
}

# rights

#In this file lies the real permissions of a process running in su

suBind() {
  #Allow to override /system/xbin/su
  allow su_daemon su_exec "file" "mounton read"

  #We will create files in /dev/su/, they will be marked as su_device
  allow su_daemon su_device "dir file lnk_file" "*"
  allow su_daemon su_device "file" "relabelfrom"
  allow su_daemon system_file "file" "relabelto"
}

#This is the vital minimum for su to open a uid 0 shell
suRights() {

  #Admit su_daemon is meant to be god.
  allow su_daemon su_daemon "capability" "sys_admin"

  allow servicemanager su "dir" "search read"
  allow servicemanager su "file" "open read"
  allow servicemanager su "process" "getattr"
  allow servicemanager su "binder" "transfer"
  [ "$API" -ge 20 ] && allow system_server su binder "call"
}

suL9() {
  allow su_daemon su_daemon "dir file lnk_file" "*"
  allow su_daemon system_data_file "dir file lnk_file" "*"
  allow su_daemon "labeledfs" filesystem "associate"
  allow su_daemon su_daemon process setfscreate
  allow su_daemon tmpfs filesystem associate
  allow su_daemon su_daemon file relabelfrom
  allow su_daemon system_file file mounton
}

otherToSU() {
  # allowLog
  if allow su logd unix_dgram_socket "sendto";then
    allow logd su dir "search"
    allow logd su file "read open getattr"
  fi

  # suBackL0
  [ "$API" -ge 20 ] && allow system_server su binder "call transfer"
  #ES Explorer opens a sokcet
  allow untrusted_app su unix_stream_socket "$rw_socket_perms connectto"
  #Any domain is allowed to send su "sigchld"
  #TODO: Have sepolicy-inject handle that
  #allow "=domain" su process "sigchld"
  allow surfaceflinger su "process" "sigchld"

  # suNetworkL0
  $BINDIR/sepolicy-inject -a netdomain -s su -P sepolicy
  $BINDIR/sepolicy-inject -a bluetoothdomain -s su -P sepolicy

  # suBackL6
  #Used by CF.lumen (restarts surfaceflinger, and communicates with it)
  #TODO: Add a rule to enforce surfaceflinger doesn't have dac_override
  allow surfaceflinger app_data_file "dir file lnk_file" "*"
  $BINDIR/sepolicy-inject -a mlstrustedsubject -s surfaceflinger -P sepolicy
}

#Samsung specific
#Prevent system from loading policy
if $BINDIR/sepolicy-inject -e -s knox_system_app -P sepolicy;then
  $BINDIR/sepolicy-inject --not -s init -t kernel -c security -p load_policy -P sepolicy
  for i in policyloader_app system_server system_app installd init ueventd runas drsd debuggerd vold zygote auditd servicemanager itsonbs commonplatformappdomain;do
    $BINDIR/sepolicy-inject --not -s "$i" -t security_spota_file -c dir -p read,write -P sepolicy
    $BINDIR/sepolicy-inject --not -s "$i" -t security_spota_file -c file -p read,write -P sepolicy
  done
fi

#Create domains if they don't exist
$BINDIR/sepolicy-inject -z su -P sepolicy
$BINDIR/sepolicy-inject -z su_device -P sepolicy
$BINDIR/sepolicy-inject -z su_daemon -P sepolicy

#Autotransition su's socket to su_device
$BINDIR/sepolicy-inject -s su_daemon -f device -c file -t su_device -P sepolicy
$BINDIR/sepolicy-inject -s su_daemon -f device -c dir -t su_device -P sepolicy
allow su_device tmpfs filesystem "associate"

#Transition from untrusted_app to su_client
#TODO: other contexts want access to su?
allowSuClient shell
allowSuClient untrusted_app
allowSuClient platform_app
allowSuClient su

#HTC Debug context requires SU
$BINDIR/sepolicy-inject -e -s ssd_tool -P sepolicy && allowSuClient ssd_tool

#Allow init to execute su daemon/transition
allow init su_daemon process "transition"
noaudit init su_daemon process "rlimitinh siginh noatsecure"
suDaemonRights
suBind

suRights
otherToSU

#Need to set su_device/su as trusted to be accessible from other categories
$BINDIR/sepolicy-inject -a mlstrustedobject -s su_device -P sepolicy
$BINDIR/sepolicy-inject -a mlstrustedsubject -s su_daemon -P sepolicy
$BINDIR/sepolicy-inject -a mlstrustedsubject -s su -P sepolicy

suL9

# Just in case :)
$BINDIR/sepolicy-inject -Z su -P sepolicy
