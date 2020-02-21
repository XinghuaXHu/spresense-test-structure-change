#!/bin/sh

WD=`pwd`
configdir=${WD}/../../../../sony_apps/test/internal_test/audio/configs

unset listing
appdir=${WD}/../../../../sony_apps
topdir=${WD}/../../../../nuttx

if [ "`basename $PWD`" != "audio" ]; then
    echo Please run under the top of audio directory.
    exit
fi

while [ ! -z "$1" ]; do
    case "$1" in
        -d )
            set -x
            ;;
        -h )
            echo "$USAGE"
            exit 0
            ;;
        -l )
            listing=y
            break
            ;;
        *)
            break
            ;;
    esac
    shift
done

if [ "X${listing}" = "Xy" ]; then
    configfiles=`ls ${configdir}/*/*-defconfig`

    for c in ${configfiles}; do
        d=`dirname $c`
        boardname=`basename $d`
        configname=`basename $c | sed -e "s/-defconfig//"`
        echo ${boardname}/${configname}
    done

    exit
fi

configfile="${configdir}/$1-defconfig"

if [ ! -f $configfile ]; then
    echo Configuration $1 not found.
    exit 1
fi

install -m 644 $appdir/scripts/Make.defs.template $topdir/Make.defs ||
    { echo "Failed to copy $appdir/scripts/Make.defs"; exit 2; }
install -m 644 $configfile $topdir/.config ||
    { echo "Failed to copy $configfile"; exit 3; }

# preconfig

make -C "${appdir}" TOPDIR=${topdir} preconfig >/dev/null

(cd ${topdir}; \
 APPSDIR="${appdir}" kconfig-conf --olddefconfig Kconfig)
