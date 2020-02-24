/****************************************************************************
 * dnnrt_lenet/dnnrt_params_def.h
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

#ifndef _DNNRT_PARAMS_DEF_H_
#  define _DNNRT_PARAMS_DEF_H_

#define DNNRT_INPUT_NUM       1   /* input layer */
#define DNNRT_BIT_SZ          28  /* dot */
#define DNNRT_INPUT_SIZE      784 /* 1*28*28 */
#define DNNRT_INPUT_NDIM      3
#define DNNRT_INPUT_SHAPE_1   1
#define DNNRT_INPUT_SHAPE_2   28
#define DNNRT_INPUT_SHAPE_3   28

#define DNNRT_OUTPUT_NUM      1
#define DNNRT_OUTPUT_SIZE     10
#define DNNRT_OUTPUT_NDIM     1
#define DNNRT_OUTPUT_SHAPE_1  10

#define DNNRT_CHK_ERROR	      -1

#define DNN_DFLT_CPUNUM       1 /* default cpu_num */
#define DNN_MAX_CPUNUM        5 /* max number of cpu */
#endif    /* _DNNRT_PARAMS_DEF_H_ */
