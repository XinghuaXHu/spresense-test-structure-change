#!/bin/sh
export LANG=en_US
PW="spresense"
#use visudo

#0V-first-step
expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for autotest: \"
	send \"${PW}\n\"
	expect \"KIKUSUI ELECTRONICS CORP.,PIA4850,WF002924,2.22\"
	expect \"> \"

	send \"*RST@@\n\"
	expect \"> \"

	send \"\[BOARD? 5\n\"

	expect \"0023\"
	send \"\[NODE 5\n\"
	expect \"> \"
	send \"\[EEPR? 1,2\n\"
	expect \"> \"
	expect \"023B\"
	send \"NODE 5\n\"
	expect \"> \"
	send \"CH 1\n\"
	expect \"> \"
	send \"OUT 0\n\"
	expect \"> \"
	send \"quit\"
	exit 0
";

