#!/bin/sh
host="43.30.157.175"

expect -c "
   spawn telnet ${host}
   expect \"Escape character is '^]'.\"
   send \"\r\"
   expect \"LABSATV3 >\"
   send \"PLAY:FILE:Shinjuku_Walk_151006\"
   send \"\r\"
   expect \"LABSATV3 >\"
	exit 0
"
