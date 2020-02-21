
Usage of audio_recorder
===========================

Usage
---------------------------

Use examples/audio_recorder default configuration

$ ./tools/config.py examples/audio_recorder -m

Select options in below.

- [Examples]
    [Audio recorder example] <= N
- [Test]
    [SQA]
      [Single Function]
        [Audio recorder test (Hi-Res)] <= Y

Build and install
--------------------------

Type 'make' to build SDK.
Install 'nuttx.spk' to system.

After that, you can see worker binary 'MP3ENC', 'SRC'
in directory sdk/modules/audio/dsp.
Store worker binary in the path specified by option.
 - Default path
    worker binary : /mnt/sd0/BIN

Recording data is written to the optional path.
 - Default path
    contents      : /mnt/sd0/REC

Execute
--------------------------

Type 'recorder_hi' on nsh.
nsh>recorder_hi

Audio from the microphone is recorded in the WAV file for 10 seconds.


