test/sqa/combination/camera_jpgdec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Description:
  Camera+JpegDecoder+LCD combination test application.

Build:
tools/config.py examples/camera examples/jpeg_decode device/lcd -m

> Examples
[ ] Camera example
[ ] JPEG decode example

> Test > SQA > Combination
[*] Test camera and jpegdecoder

Run:
camera_jpgdec

Options:
 -w <width>  : Picture width.
 -h <height> : Picture height.
 -c <count>  : Repeat count.
 -m          : Use mcu.
 -i          : Lcd show interval (sec)
