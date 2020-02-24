# examples/dnnrt_multi

SYNOPSIS
       dnnrt_multi [-s] [nnb] [pgm] [cpuno]

DESCRIPTION
       dnnrt_multi is for a test using multi core
       with model defined by nnb (default value: /mnt/sd0/lenet-5/model/lenet-5.nnb),
       and feeds an image of pgm (default value: /mnt/sd0/lenet-5/data/0.pgm).
       Default path is used if no setting of nnb or pgm parameter.

OPTIONS
       cpuno [1-5] 
       no.1 is used in default if no choice. 

### expected output:

nsh> dnnrt_multi /mnt/sd0/lenet-5.nnb /mnt/sd0/3.pgm
load nnb file: /mnt/sd0/lenet-5.nnb
load pnm image: /mnt/sd0/3.pgm # 3 is hand-written
normalization: divide image data by 255.0 # normalization is done in the application-side
...

nsh> dnnrt_multi /mnt/sd0/lenet-5.nnb /mnt/sd0/3.pgm 4
Load nnb file: /mnt/sd0/lenet-5.nnb
Load pgm image: /mnt/sd0/3.pgm
set cpu_num: : 4
Image Normalization (1.0/255.0): enabled
start dnn_runtime_forward()
output[0]=0.000000
output[1]=0.000000
output[2]=0.000000
output[3]=1.000000 # probability that 3 is written in the given image
output[4]=0.000000
output[5]=0.000000
output[6]=0.000000
output[7]=0.000000
output[8]=0.000000
output[9]=0.000000
inference time=0.042
...
```

a check point is a value of "inference time" after executed.
this value as result ill be less than one proceeded at the single core 
when a test executed correctly at the multi core.
