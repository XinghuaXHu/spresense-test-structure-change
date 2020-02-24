#!/bin/sh
host="43.30.157.175"

expect -c "
   spawn telnet ${host}
   expect \"Escape character is '^]'.\"
   send \"\r\"
   expect \"LABSATV3 >\"
   send \"PLAY:FILE:Ebina_taka_20140926\"
   send \"\r\"
   expect eof
"
python3 gnss15677_test_tc_runner.py -v &

for i in `seq 1 5`
do
  expect -c "
     spawn telnet ${host}
     expect \"Escape character is '^]'.\"
     send \"\r\"
     expect \"LABSATV3 >\"
     send \"MUTE:Y\r\"
     send \"\r\"
     expect eof
	";
	sleep 60;

  expect -c "
     spawn telnet ${host}
     expect \"Escape character is '^]'.\"
     send \"\r\"
     expect \"LABSATV3 >\"
     send \"MUTE:N\r\"
     send \"\r\"
     expect eof
	";
  sleep 60;

done

expect -c "
   spawn telnet ${host}
   expect \"Escape character is '^]'.\"
   send \"\r\"
   expect \"LABSATV3 >\"
   send \"PLAY:STOP\"
   send \"\r\"
   expect eof
"

