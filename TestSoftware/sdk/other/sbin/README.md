# Secure binary grouping test

This directory contains group loading test, both of binary cloning and combined binaries.

## Prepare for test

Please install all of .espk files in espks directory.

```
$ cd sdk
$ ./tools/flash.sh ../test/sbin/espks/test1.espk
$ ./tools/flash.sh ../test/sbin/espks/test2.espk
$ ./tools/flash.sh ../test/sbin/espks/test3.espk
$ ./tools/flash.sh ../test/sbin/espks/test4.espk
$ ./tools/flash.sh ../test/sbin/espks/test5.espk
```

test?.epsk is a combined binary, except test1.espk.
These files were created by SonySemiconductorSolutions/spresense-secbin repository,
They are just blink LED against running CPU ID.
(Unfortunately, CPU5 is none, so test program omit 5th)

Each espk files created by:

```
$ mkespk compose -k <keyfile> test1.espk dsp
$ mkespk compose -k <keyfile> test2.espk dsp dsp
$ mkespk compose -k <keyfile> test3.espk dsp dsp dsp
$ mkespk compose -k <keyfile> test4.espk dsp dsp dsp dsp
$ mkespk compose -k <keyfile> test5.espk dsp dsp dsp dsp dsp
```

`dsp` is an ELF file created by SonySemiconductorSolutions/spresense-secbin repository.
(Path to mkepsk and keyfile are omitted.)

As shown as above, all of espk files are created from same ELF file, and number of file name means number of combined binaries.

## Configuration and build

```
$ ./tools/config.py -d ../test/sbin
$ make
```

## Run

Flash `nuttx.spk` and type command at nsh console.

```
nsh> sbin
```

You can see the LEDs blinking and increase blinking LEDs.

### Log

```
nsh> sbin

Start test for unified binary

Assigned CPUs: 3
test1 is running a while.. done
Assigned CPUs: 3 4
test2 is running a while.. done
Assigned CPUs: 3 4 5
test3 is running a while.. done
Assigned CPUs: 3 4 5 6
test4 is running a while.. done

Start test for cloning

Assigned CPUs: 3
test1 is running a while.. done
Assigned CPUs: 3 4
test1 is running a while.. done
Assigned CPUs: 3 4 5
test1 is running a while.. done
Assigned CPUs: 3 4 5 6
test1 is running a while.. done
```
