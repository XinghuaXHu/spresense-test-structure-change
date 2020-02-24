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

typedef struct mq_data {
  char msg[MSG_DATA_SZ];
}mq_data_t;

typedef struct sec_task {
  /* data area for test */
  mptask_t  task;
  mpmutex_t *csem;
  mpmutex_t sem; 
  mpmq_t    mq;
  mpshm_t   shm;
  void      *buf;
}sec_task_t;

typedef struct sec_info {
  char fullpath3[SEC_MAXFULLPATH];
  char fullpath4[SEC_MAXFULLPATH];
  sec_task_t s_task[MAXTASKS];
  int tile_start;
  int ac_size;
  int cpucnt;
 
  /* values for a common resource */ 
  mpmutex_t sem;
  mpmq_t    mq;
  mpshm_t   shm;
}sec_info_t; 
