#!/bin/sh
host="43.30.157.175"

expect -c "
   spawn telnet ${host}
   expect \"Escape character is '^]'.\"
   send \"\r\"
   expect \"LABSATV3 >\"
   send \"PLAY:STOP\"
   send \"\r\"
   expect \"LABSATV3 >\"
	exit 0
" > /dev/null
