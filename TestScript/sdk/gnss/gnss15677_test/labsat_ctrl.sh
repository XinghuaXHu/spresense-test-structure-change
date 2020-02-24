#!/bin/sh
host="43.30.157.175"

for i in `seq 1 5`
do
  expect -c "
     spawn telnet ${host}
     expect \"Escape character is '^]'.\"
     send \"\r\"
     expect \"LABSATV3 >\"
     send \"MUTE:Y\r\"
     send \"\r\"
     expect \"LABSATV3 >\"
	exit 0
	";
	sleep 60;

  expect -c "
     spawn telnet ${host}
     expect \"Escape character is '^]'.\"
     send \"\r\"
     expect \"LABSATV3 >\"
     send \"MUTE:N\r\"
     send \"\r\"
     expect \"LABSATV3 >\"
	exit 0
	";
  sleep 60;

done
