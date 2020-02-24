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
python3 gnss_factory_test_tc_runner.py -v ;

expect -c "
   spawn telnet ${host}
   expect \"Escape character is '^]'.\"
   send \"\r\"
   expect \"LABSATV3 >\"
   send \"PLAY:STOP\"
   send \"\r\"
   expect eof
"

