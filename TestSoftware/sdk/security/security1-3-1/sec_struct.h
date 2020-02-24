/****************************************************************************
 * test/sqa/singlefunction/multicpu/security/multicpu/sec_struct.h
 *
 *   Copyright (C) 2019 Sony Corporation
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
 * 3. Neither the name NuttX nor Sony nor the names of its contributors
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

/**
 *  original data structure for mq
 *  paddr : physical address of a data areq in mq 
 */ 

typedef struct sec_mq_dtype{
  uintptr_t paddr;
  uintptr_t wpaddr;
  size_t    size;
  int       data; 
}sec_mq_dtype_t;
  
typedef struct mq_data {
  char msg[MSG_DATA_SZ];
}mq_data_t;

typedef struct sec_combinfo {
  mptask_t   task; /* fot a secure task test combined */
  int        cpus;
  cpu_set_t  cpuids;
  mpmutex_t  mutex;
  
}sec_combinfo_t;

#if 0
typedef struct sec_task {
  /* data area for test */
  mq_data_t tdata;
  int taskno;
  sigset_t  sig_d;
  mptask_t  task;
  mpmutex_t *csem;
  mpmutex_t sem; 
  mpmq_t    mq;
  mpshm_t   shm;
  void      *buf;
}sec_task_t;
#endif 

typedef struct sec_etask {
  /* data area for test */
  mq_data_t tdata;
  int cpuno;
  sigset_t  sig_d;
  mptask_t  task;
  mpmq_t    mq;
  mpshm_t   shm;
  char      shm_allc;  
  sec_mq_dtype_t dtype;
  void      *buf;
}sec_etask_t;

typedef struct sec_info {
  char fullpath[SEC_MAXFULLPATH];
  sec_etask_t s_etask[SEC_MAXTASKS];
  sec_combinfo_t s_comb;
 
  /* values for a common resource */ 
  sigset_t  sig_d;
  sigset_t  th_sig_d;
  mpmutex_t sem;
  mpmq_t    mq;
  mpshm_t   shm;
  pid_t     my_pid; /* main task process */ 
  pid_t     th_pid; /* thread process */
}sec_info_t; 
