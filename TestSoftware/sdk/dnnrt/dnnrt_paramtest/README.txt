# examples/dnnrt_paramtest

SYNOPSIS
       dnnrt_paramtest

DESCRIPTION
       dnnrt_paramtest is to check some parameters for input and output in a neural network model. 
OPTIONS

### expected output:

nsh> dnnrt_paramtest
Load nnb file: /mnt/sd0/lenet-5.nnb
Load pgm image: /mnt/sd0/0.pgm
Image Normalization (1.0/255.0): enabled

start checking inputval! 
dnn_runtime_input_num():1 check OK 
dnn_runtime_input_size():784 check OK 
dnn_runtime_input_ndim():4 check OK 
m:0 dnn_runtime_input_shape():1 check OK 
m:1 dnn_runtime_input_shape():1 check OK 
m:2 dnn_runtime_input_shape():28 check OK 
m:3 dnn_runtime_input_shape():28 check OK 

start checking outputval! 
dnn_runtime_output_num():1 check OK 
dnn_runtime_output_size():10 check OK 
dnn_runtime_output_ndim():2 check OK 
m:0 dnn_runtime_output_shape():1 check OK 
m:1 dnn_runtime_output_shape():10 check OK 

SUCCEDED check_parm finished!!


RESULTS
       SUCCEDED : all tests finished successfully.
       FAILED   : a part of test or all tests finished with NG. 

...
```

