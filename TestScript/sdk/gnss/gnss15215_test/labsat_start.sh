#!/bin/sh
host="43.30.157.175"

RESULT="NG"
RETRY=0

while [ "${RESULT}" = "NG" ]
do
   LABSAT_OUT=`expect -c "
      spawn telnet ${host}
      expect \"Escape character is '^]'.\"
      send \"\r\"
      expect \"LABSATV3 >\"
      send \"PLAY:FILE:Ebina_taka_20140926\"
      send \"\r\"
      expect \"LABSATV3 >\"
      exit 0
   "`

   echo ${LABSAT_OUT} | sed "s/\r/\n/g"

   OK=`echo ${LABSAT_OUT} | sed "s/\r/\n/g" | grep "OK"`
   if [ "${OK}" != "" ]; then
      RESULT="OK"
   else
      RESULT="NG"
      RETRY=`expr ${RETRY} + 1`
      if [ ${RETRY} -lt 10 ]; then
         echo "ERROR: LABSAT Failed retry ${RETRY}..."
         sleep 1
         ./labsat_end.sh
      else
         echo "Retry count expire."
         exit 1
      fi
   fi
done
