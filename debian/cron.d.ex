#
# Regular cron jobs for the youki package
#
0 4	* * *	root	[ -x /usr/bin/youki_maintenance ] && /usr/bin/youki_maintenance
