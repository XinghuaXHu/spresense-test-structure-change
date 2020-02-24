#!/bin/sh
export LANG=en_US
PW="spresense"
#use visudo
device=$1
usbpath=$2


expect -c "
    spawn sudo umount ${usbpath}
    expect \"password for\"
    send \"${PW}\n\"
    exit 0
";

expect -c "
    spawn sudo umount ${usbpath}
    expect \"password for\"
    send \"${PW}\n\"
    exit 0
";

umount /media/$LOGNAME/*
sleep 5

mkdir -p $usbpath
mountlist=`mount | grep ${device}`

if [ -z "$mountlist" ];then
	expect -c "
		spawn sudo mount ${device} ${usbpath} -o umask=0
		expect \"password for\"
		send \"${PW}\n\"
		sleep 5
		exit 0
	";
	sleep 5
fi

