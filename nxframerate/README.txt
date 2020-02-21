
NX graphics library and LCD device framerate measurement test
===========================

Configuration
---------------------------

$ make buildkernel KERNCONF=lcd
$ ./tools/config.py -m ili9340

And enable this test from menuconfig.


Usage
---------------------------

Type following command from nsh console:

$ nxframerate

And prints elapsed time for drawing fully LCD area in 100 times.
Please calculate its time to FPS by following formula:

FPS = 1 / (<time> / 100)
