#!/bin/bash
mv ../../../../../examples/gnss/gnss_main.c ../../../../../gnss_main.c.def
cp ./gnss_main.c ../../../../../examples/gnss/
cd ../../../../../sdk/
./tools/config.py examples/gnss
make clean
make
./tools/flash.sh -c /dev/ttyUSB0 nuttx.spk
cd ../test/autotest/src/tc/system14964_test
python3 system14964_test_tc_runner.py -s
