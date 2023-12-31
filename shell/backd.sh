#!/bin/sh
#
# backd     This shell script takes care of starting and stopping
#             backd daemon.
#
# chkconfig: 2345 95 05
#
# description: backd is a daemon to do backup process automaticaly
# 	       (or almost)
# Source function library.
. /etc/rc.d/init.d/functions

BACKDBASE=/var/lib/backd

if [ -f /etc/sysconfig/backd ]; then
 . /etc/sysconfig/backd
fi

# See how we were called.
case "$1" in
  start)
	# Start daemons.
	echo -n "Starting backd daemon in background mode: "
	umask 022
	$BACKDBASE/bin/backd -b $BACKDBASE > /dev/null 2>&1
	daemon /bin/true
	touch /var/lock/subsys/backd
	echo
	;;
  stop)
	# Stop daemons.
	echo -n "Shutting down backd: "
	killproc backd
	echo
	rm -f /var/lock/subsys/backd
	;;
  restart)
	$0 stop
	$0 start
	;;
  status)
	status backd
	;;
  *)
	echo "Usage: backd {start|stop|restart|status}"
	exit 1
esac

exit 0

