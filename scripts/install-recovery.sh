#!/system/bin/sh
##########################################################################################
#
# Magisk daemon trigger
# by Afaneh92
#
##########################################################################################

# The following commands should run on each reboot
/system/xbin/magiskpolicy --live --magisk "allow magisk * * *"
/system/xbin/magisk --daemon
