#!/bin/sh
export LANG=en_US
PW="top"

#0V-first-step
expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for top: \"
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

sleep 10;

#5V second-step
expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for top: \"
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
	send \"VSET 5.00000\n\"
	expect \"> \"
	send \"ISET 0.50000\n\"
	expect \"> \"
	send \"OUT 1\n\"
	expect \"> \"
	send \"quit\"
	exit 0
";

sleep 10;

#1.8V second-step
expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for top: \"
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
	send \"VSET 1.80000\n\"
	expect \"> \"
	send \"ISET 0.50000\n\"
	expect \"> \"
	send \"OUT 1\n\"
	expect \"> \"
	send \"quit\"
	exit 0
";

sleep 10;

#1.44V therd-step
expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for top: \"
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
	send \"VSET 1.44000\n\"
	expect \"> \"
	send \"ISET 0.50000\n\"
	expect \"> \"
	send \"OUT 1\n\"
	expect \"> \"
	send \"quit\"
	exit 0
";

sleep 10;

#1.0V forth-step
expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for top: \"
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
	send \"VSET 1.00000\n\"
	expect \"> \"
	send \"ISET 0.50000\n\"
	expect \"> \"
	send \"OUT 1\n\"
	expect \"> \"
	send \"quit\"
	exit 0
";

sleep 10;

expect -c "
	spawn sudo usbtmc-shell --backend linux_kernel /dev/usbtmc0
	expect \"password for top: \"
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
