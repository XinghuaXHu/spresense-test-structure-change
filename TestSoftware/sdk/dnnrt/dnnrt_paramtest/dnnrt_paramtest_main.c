
/****************************************************************************
 * dnnrt_test1/dnnrt_paramtest.c
 *
 *   Copyright 2018 Sony Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <nuttx/config.h>
#include <dnnrt/runtime.h>
#include "loader_nnb.h"
#include "pnm_util.h"
#include "dnnrt_params_def.h"
#include "dnnrt_params_prot.h"

/****************************************************************************
 * Type Definition
 ****************************************************************************/
typedef struct
{
  char *nnb_path;
  char *pgm_path;
  bool skip_norm;
} my_setting_t;

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define DNN_PNM_PATH    "/mnt/sd0/0.pgm"
#define DNN_NNB_PATH    "/mnt/sd0/lenet-5.nnb"
#define MNIST_SIZE_PX (28*28)

/****************************************************************************
 * Private Data
 ****************************************************************************/
static float s_img_buffer[MNIST_SIZE_PX];

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static void parse_args(int argc, char *argv[], my_setting_t * setting)
{
  /* parse options by getopt() */
  int opt;
  while ((opt = getopt(argc, argv, "s")) != -1)
    {
      switch (opt)
        {
        case 's':              /* skip normalization */
          setting->skip_norm = true;
          break;
        }
    }

  /* set my_setting_t::{nnb_path,pgm_path} to argv[] if necessary */
  setting->nnb_path = (optind < argc) ? argv[optind++] : DNN_NNB_PATH;
  setting->pgm_path = (optind < argc) ? argv[optind] : DNN_PNM_PATH;

  /* print my_setting_t */
  printf("Load nnb file: %s\n", setting->nnb_path);
  printf("Load pgm image: %s\n", setting->pgm_path);
  if (setting->skip_norm)
    {
      printf("Image Normalization (1.0/255.0): skipped\n");
    }
  else
    {
      printf("Image Normalization (1.0/255.0): enabled\n");
    }
}
static int dnn_check_inputval(dnn_runtime_t *rtd)
{
  int ret;
  int res = DNNRT_E_OK;
  int k;
  int m;
  int expectedVal=0; 

  printf("\nstart checking inputval! \n");
  /* check params -- input_num -- */
  ret = dnn_runtime_input_num(rtd);
  if(ret < 0)
    {
      printf("dnn_runtime_input_num() failed due to %d\n", ret);
      res = DNNRT_E_FUNC;
      goto chk_error;
    }
  else
    {
      if(ret == DNNRT_INPUT_NUM)
        {
          printf("dnn_runtime_input_num():%d check OK \n",ret);
        }
      else
       {
          printf("dnn_runtime_input_num():%d check NG \n",ret);
          res = DNNRT_E_FAIL;
          goto chk_error;
       }
    }
  
  for(k=0; k<DNNRT_INPUT_NUM; k++){ 
    /* check params -- input_size -- */
    ret = dnn_runtime_input_size(rtd, k);
    if(ret < 0)
      {
        printf("dnn_runtime_input_size() failed due to %d\n", ret);
        res = DNNRT_E_FUNC;
        goto chk_error;
      }
    else
      {
        if(ret == DNNRT_INPUT_SIZE)
          {
            printf("dnn_runtime_input_size():%d check OK \n",ret);
          }
        else
          {
            printf("dnn_runtime_input_size():%d check NG \n",ret);
            res = DNNRT_E_FAIL;
            goto chk_error;
          }
       }
    }
  
  for(k=0; k<DNNRT_INPUT_NUM; k++)
   { 
     /* check params -- input_ndim -- */
     ret = dnn_runtime_input_ndim(rtd, k);
     if(ret < 0)
       {
         printf("dnn_runtime_input_ndim() failed due to %d\n", ret);
         res = DNNRT_E_FUNC;
         goto chk_error;
       }
     else
       {
         if(ret == DNNRT_INPUT_NDIM)
           {
             printf("dnn_runtime_input_ndim():%d check OK \n",ret);
           }
         else
           {
             printf("dnn_runtime_input_ndim():%d check NG \n",ret);
             res = DNNRT_E_FAIL;
             goto chk_error;
           }
       }
 
     for(m=0; m<DNNRT_INPUT_NDIM; m++)
       { 
         /* check params -- input_shape -- */
         ret = dnn_runtime_input_shape(rtd, k, m);
         if(ret < 0)
           {
             printf("dnn_runtime_input_shape() failed due to %d\n", ret);
             res = DNNRT_E_FUNC;
             goto chk_error;
           }
         else
           { 
           switch (m)
             {
               case 0:
                 expectedVal = DNNRT_INPUT_SHAPE_0; 
                 break;
               case 1:
                 expectedVal = DNNRT_INPUT_SHAPE_1; 
                 break;
               case 2:
                 expectedVal = DNNRT_INPUT_SHAPE_2; 
                 break;
               case 3:
                 expectedVal = DNNRT_INPUT_SHAPE_3; 
                 break;
               default:
                 printf("internal loop_error %d \n", m);
                 res = DNNRT_E_PARAM;
                 goto chk_error;
             } 
           if(ret == expectedVal)
             {
               printf("m:%d dnn_runtime_input_shape():%d check OK \n",m, ret);
             }
           else
             {
               printf("m:%d dnn_runtime_input_shape():%d check NG \n",m, ret);
               res = DNNRT_E_FAIL;
               goto chk_error;
             }
          }
       }
    }
chk_error:
  return res;
}

static int dnn_check_outputval(dnn_runtime_t *rtd)
{
  int ret;
  int res=DNNRT_E_OK;
  int k;
  int m;
  int expectedVal=0; 
  
  printf("\nstart checking outputval! \n");

  /* check params -- output_num -- */
  ret = dnn_runtime_output_num(rtd);
  if(ret < 0)
    {
      printf("dnn_runtime_output_num() failed due to %d\n", ret);
      res = DNNRT_E_FUNC;
      goto chk_error;
    }
  else
    {
      if(ret == DNNRT_OUTPUT_NUM)
        {
          printf("dnn_runtime_output_num():%d check OK \n",ret);
        }
      else
       {
          printf("dnn_runtime_output_num():%d check NG \n",ret);
          res = DNNRT_E_FAIL;
          goto chk_error;
       }
    }
  
  for(k=0; k<DNNRT_OUTPUT_NUM; k++){ 
    /* check params -- output_size -- */
    ret = dnn_runtime_output_size(rtd, k);
    if(ret < 0)
      {
        printf("dnn_runtime_output_size() failed due to %d\n", ret);
        res = DNNRT_E_FUNC;
        goto chk_error;
      }
    else
      {
        if(ret == DNNRT_OUTPUT_SIZE)
          {
            printf("dnn_runtime_output_size():%d check OK \n",ret);
          }
        else
          {
            printf("dnn_runtime_output_size():%d check NG \n",ret);
            res = DNNRT_E_FAIL;
            goto chk_error;
          }
       }
    }
  
  for(k=0; k<DNNRT_OUTPUT_NUM; k++)
   { 
     /* check params -- output_ndim -- */
     ret = dnn_runtime_output_ndim(rtd, k);
     if(ret < 0)
       {
         printf("dnn_runtime_output_ndim() failed due to %d\n", ret);
         res = DNNRT_E_FUNC;
         goto chk_error;
       }
     else
       {
         if(ret == DNNRT_OUTPUT_NDIM)
           {
             printf("dnn_runtime_output_ndim():%d check OK \n",ret);
           }
         else
           {
             printf("dnn_runtime_output_ndim():%d check NG \n",ret);
             res = DNNRT_E_FAIL;
             goto chk_error;
           }
       }
 
     for(m=0; m<DNNRT_OUTPUT_NDIM; m++)
       { 
         /* check params -- output_shape -- */
         ret = dnn_runtime_output_shape(rtd, k, m);
         if(ret < 0)
           {
             printf("dnn_runtime_output_shape() failed due to %d\n", ret);
             res = DNNRT_E_FUNC;
             goto chk_error;
           }
         else
           { 
           switch (m)
             {
               case 0:
                 expectedVal = DNNRT_OUTPUT_SHAPE_0; 
                 break;
               case 1:
                 expectedVal = DNNRT_OUTPUT_SHAPE_1; 
                 break;
               default:
                 printf("internal loop_error %d \n", m);
                 res = DNNRT_E_PARAM;
                 goto chk_error;
             } 
           if(ret == expectedVal)
             {
               printf("m:%d dnn_runtime_output_shape():%d check OK \n",m, ret);
             }
           else
             {
               printf("m:%d Ndnn_runtime_output_shape():%d check NG \n",m, ret);
               res = DNNRT_E_FAIL;
               goto chk_error;
             }
          }
       }
    }
chk_error:
  return res;

}


/****************************************************************************
 * dnnrt_paramtest_main
 ****************************************************************************/
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int dnnrt_paramtest_main(int argc, char *argv[])
#endif
{
  int ret;
  float norm_factor;
  dnn_runtime_t rt;
  dnn_config_t config = {.cpu_num = 1 };
  nn_network_t *network;
  my_setting_t setting = { 0 };

  parse_args(argc, argv, &setting);

  /* load an hand-written digit image into s_img_buffer,
   * and then divide the pixels by 255.0 for normalization */
  norm_factor = setting.skip_norm ? 1.0f : 255.0f;
  ret =
    pnm_load(setting.pgm_path, norm_factor, s_img_buffer, sizeof(s_img_buffer));
  if (ret)
    {
      printf("load pgm image failed due to %d\n", ret);
      goto pgm_error;
    }

  /* load an nnb file, which holds a network structure and weight values,
   * into a heap memory */
  network = alloc_nnb_network(setting.nnb_path);
  if (network == NULL)
    {
      printf("load nnb file failed\n");
      goto pgm_error;
    }

  /* Step-A: initialize the whole dnnrt subsystem */
  ret = dnn_initialize(&config);
  if (ret)
    {
      printf("dnn_initialize() failed due to %d", ret);
      goto dnn_error;
    }

  /* Step-B: instantiate a neural network defined
   * by nn_network_t as a dnn_runtime_t object */
  ret = dnn_runtime_initialize(&rt, network);
  if (ret)
    {
      printf("dnn_runtime_initialize() failed due to %d\n", ret);
      goto rt_error;
    }

  ret = dnn_check_inputval(&rt);
  if (ret)
    {
      switch(ret){
        case DNNRT_E_FAIL:
          printf("Error an unexpected value of return\n");
          break;
        case DNNRT_E_FUNC:
          printf("Error a value of return in api\n");
          break;
        case DNNRT_E_PARAM:
        default:
          printf("Error an abnormal return value! \n");
      }
      goto rt_error;
    }
  
  ret = dnn_check_outputval(&rt);
  if (ret)
    {
      switch(ret){
        case DNNRT_E_FAIL:
          printf("Error an unexpected value of return\n");
          break;
        case DNNRT_E_FUNC:
          printf("Error a value of return in api\n");
          break;
        case DNNRT_E_PARAM:
        default:
          printf("Error an abnormal return value! \n");
      }
      goto rt_error;
    }
  printf("\nSUCCEDED check_parm finished!!\n");
  /* Step-F: free memories allocated to dnn_runtime_t */
  dnn_runtime_finalize(&rt);
rt_error:
  /* Step-G: finalize the whole dnnrt subsystem */
  dnn_finalize();
dnn_error:
  /* just call free() */
  destroy_nnb_network(network);
pgm_error:
  return ret;
}
