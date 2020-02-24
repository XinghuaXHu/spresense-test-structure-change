
Usage of audio_recognizer
===========================

Usage
---------------------------

Use a defconfig of audio_recog2.
$ ./tools/config.py examples/audio_recog2

Select options in below.

- [CXD56xx Configuration]
    [SDIO SD Card] <= Y
    [Audio] <= Y
- [SDK audio] <= Y
    [Audio Utilities]
      [Audio Recognizer] <= Y
- [Memory manager] <= Y
- [ASMP] <= Y
- [Test]
    [SQA]
      [SingleFunction]
        [Audio recognizer2 example] <= Y


Build and install
--------------------------

Type 'make' to build SDK.
Install 'nuttx.spk' to system.

After that, you can see worker binary 'RCGPROC', 'RCGPROC2'
in directory worker_recognizer/worker_recognizer2.
Store worker binary in the path specified by option.
 - Default path
    worker binary : /mnt/sd0/BIN

NOTE

  Build preprocess DSP and deploy
  --------------------------
  
  If you would like to use preprocess DSP.
  Set config "SQAT_AUDIO_RECOG2_USEPREPROC" = "y" before do make.

  When SQAT_AUDIO_RECOG2_USEPREPROC is set and build this example,
  DSP binary for preprocess will out at "worker_preprocess/PREPROC".
  Please place PREPROC binary file to SD card(/mnt/sd0/BIN).

  The codes of DSP is placed on "worker_preprocess/src(or include)".
  You can edit them and it will be built with framework code in sdk(*)
  and embedded to DSP binary.
  (*)The framework codes are at "sdk/module/audio/components/usercustom/dsp_framework".

 To build RCGPROC2:
 You must select the following config to create.
   [build RCGPROC2 for another dsp code] <= Y

Execute
--------------------------

Type 'audio_recognizer on nsh.
nsh>audio_recognizer <number of loop> <debug flag> <random flag>

number of loop : number for test in loop.
debug flag     : enabled to print the shared memory as DSP info on the way.    
random flag    : enabled a random interval for process in recognizer DSP.

