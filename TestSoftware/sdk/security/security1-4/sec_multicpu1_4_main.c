/****************************************************************************
 * test/sqa/singlefunction/security1-4/sec_multicpu1_4_main.c
 *
 *   Copyright (C) 2018 Sony Corporation
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/shm.h>
#include <unistd.h>

#include <nuttx/drivers/ramdisk.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <stdbool.h>

#include <asmp/asmp.h>
#include <asmp/mptask.h>
#include <asmp/mpshm.h>
#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>

#include <sys/time.h>
#include <nuttx/arch.h>

#include "sec_def.h"
#include "sec_prot.h"
#include "sec_struct.h"


//#define DEBUG_ON
//#define _USE_SIGNAL
//#define _USE_MULTI_MPMUTEX 
//#define _USE_PADDR
//#define _USE_ATTACH1
//#define _USE_ATTACH2
//#define _USE_DETACH1
//#define _USE_DETACH2
//#define _USE_WAITSIG

#ifdef _USE_SIGNAL
  #define _USE_FIRSTMSG  
//  #define _USE_WAIT_FIRSTMSG  
#endif
#define _2ND_SECGRP_ENA
/* LED check in case of test between secure core  */
#define _3RD_SECGRP_ENA

#define SEC_TH_STACKSIZE 2048 
#define SEC_TH_PRIORITY  SCHED_PRIORITY_DEFAULT 

//#define _USE_THREAD
#define _TEST_SIG
#define _ASMP_SEC 
#define _ENA_SHM
#define _PROT_TEST
#define _INTER_SUBCORE_TEST

#define putreg32(val, addr) *(volatile uint32_t *)(addr) = (val)
#define getreg32(addr)      *(volatile uint32_t *)(addr)
#define wraddr8(val, addr) *(volatile char *)(addr) = (val)
#define wraddr32(val, addr) *(volatile uint32_t *)(addr) = (val)

#define treg   *( volatile char *)0

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#ifdef CONFIG_FS_ROMFS
#  include "worker/romfs.h"

#  define SECTORSIZE   512
#  define NSECTORS(b)  (((b)+SECTORSIZE-1)/SECTORSIZE)
#  define MOUNTPT "/romfs"
#endif

#ifndef MOUNTPT
#  define MOUNTPT "/mnt/sd0/BIN"
//#  define MOUNTPT "/mnt/spif"
#endif

/* MP object keys. Must be synchronized with worker. */

#define KEY_SHM   1
#define KEY_MQ    2
#define KEY_MUTEX 3

#define MSG_ID_SAYHELLO 1
#define MSG_ID_TESTMSG  2 

#define DEBUG_LEVEL  2 
//#define DEBUG_LEVEL    4 

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#if CONFIG_NFILE_DESCRIPTORS < 1
#  error "You must provide file descriptors via CONFIG_NFILE_DESCRIPTORS in your configuration file"
#endif

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)
#define pr_debug(dbg,format, ...)   {if(dbg>=DEBUG_LEVEL) printf(format, ##__VA_ARGS__);}

#ifdef _USE_THREAD
static pthread_t monitor_th_tid;
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/
typedef struct {
  mptask_t task;
  mpmutex_t  sem;
  mpmq_t   mq;
} mptask_info_t;

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Symbols from Auto-Generated Code
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static int init_param_secure(sec_info_t *);
#ifndef _2ND_SECGRP_ENA
static int setup_task_secure(const char *, sec_info_t *);
#endif
static int setup_task_secure2(const char *, sec_info_t *, int );
static int setup_task_secure3(const char *, sec_info_t *, int );
static int finalize_task_secure(sec_info_t *);
static int finalize_task_secure3(sec_info_t *);
#ifndef _3RD_SECGRP_ENA
static int start_task_secure(sec_info_t *);
#endif
static int start_task_secure2(sec_info_t *, int );
static int start_task_secure3(sec_info_t *, int );

static int set_rootfs(sec_info_t *);
static int set_param_worker_secure(sec_info_t *);
static int send_firstmsg_secure(sec_info_t *);
#ifdef _USE_THREAD
static int set_thread(sec_info_t *);
#endif
static int init_param(sec_info_t *);
static int setup_task(const char *, sec_task_t *);
static int send_firstmsg(sec_info_t *);
static int finalize_task(sec_info_t *);
static int set_area_addr(int , char *[], sec_info_t *);
static int init_for_signal2(sec_info_t *);
#if 0 
static int set_param_worker(sec_info_t *);
#endif
#ifdef _USE_WAITSIG
static int pr_err_task(siginfo_t *);
#endif
#ifdef _TEST_NONSECURE 
static int start_task(sec_task_t *);

#ifdef _USE_SIGNAL 
static int init_for_signal(sec_info_t *);
static int unmask_signal(sec_info_t *);
#endif

#ifdef _USE_WAIT_FIRSTMSG
static int wait_firstmsg(sec_info_t *);
#endif

#ifdef _USE_WAITSIG
static int wait_signal(sec_info_t *);
#endif
//static int sec_chk_msg(uint32_t *, sec_task_t *, int);

static int sec_send_nextmsg(sec_info_t *, int , int);
static int set_signal_as_rslt(sec_info_t *, int , pid_t); 
static int set_signal_err(sec_info_t *, int , pid_t); 
static void sec_init_tdata(sec_info_t *);
static int set_char_formsg(char *, int , int);
static FAR void sec_test_monitor5(FAR void *);

#endif /* _TEST_NONSECURE */ 

#ifdef _USE_THREAD
static FAR void sec_test_monitor_sec(FAR void *);
#endif

#ifdef _USE_THREAD
static int set_signal_asrslt(sec_info_t *, pid_t); 
#endif
static int chk_mem_access(sec_info_t *);

/****************************************************************************
 * Debug Functions 
 ****************************************************************************/
void pr_dbg_initmem(sec_task_t *s_task, int sz)
{
  int m;
  char *buf;

  buf = (char *)(s_task->buf);
  pr_debug(2, "init buf:%p \n",buf); 

  pr_debug(2,"buf:%p \n",buf); 

  /* initialize a shared memory */
  for(m=0; m<sz; m++){
    buf[m] = 0x00; 
  }
  return;
}

void pr_dbg_initmem_sec(sec_etask_t *s_etask, int sz)
{
  int m;
  char *buf;

  buf = (char *)(s_etask->buf);
  pr_debug(2, "init buf:%p \n",buf); 

  pr_debug(2,"buf:%p \n",buf); 

  /* initialize a shared memory */
  for(m=0; m<sz; m++){
    buf[m] = 0x00; 
  }
  return;

}

void pr_dbg_workerinfo(sec_etask_t *s_etask, int i, int sz, int offset)
{
  char *buf;
  int m;

  pr_debug(3,"Memory for a secure worker%d\n", i);

  buf = (char *)(s_etask->buf); 
  pr_debug(2,"buf:%p \n",buf); 

  for(m=offset; m<sz; m++){
    message("%02x ",buf[m]);
 
    if(!((m+1)%16)) message("\n");
  }

  pr_debug(3,"Display %d finished \n", i); 

  return;
}
void pr_dbg_workerinfo_n(sec_task_t *s_task, int i, int sz, int offset)
{
  char *buf;
  int m;

  pr_debug(3,"Memory for a non-secure eworker%d\n", i);

  buf = (char *)(s_task->buf); 
  pr_debug(2,"buf:%p \n",buf); 

  for(m=offset; m<sz; m++){
    message("%02x ",buf[m]);
 
    if(!((m+1)%16)) message("\n");
  }

  pr_debug(3,"Display %d finished \n", i); 

  return;
}
void pr_dbg_workerinfo2(sec_mq_dtype_t *dtype, int sz, int offset)
{
  int m;

  pr_debug(3,"Shared Memory \n");

  for(m=offset; m<sz; m++){
    message("%02x ",((char*)(dtype->paddr))[m]); 
 
    if(!((m+1)%16)) message("\n");
  }

  pr_debug(3,"Display finished \n"); 

  return;
}

#ifdef DEBUG_ON
/****************************************************************************
 * Debug Functions 
 ****************************************************************************/
void pr_dbg_initmem(sec_etask_t *s_etask, int sz)
{
  int m;
  char *buf;

#ifdef _USE_ATTACH1
  buf = (char *)mpshm_attach(&s_etask->shm, 0);
  if(buf==NULL){
    err("Attach error!! \n"); 
    return;
  } 
#else
  buf = (char *)(s_etask->buf);
  pr_debug(2, "init buf:%p \n",buf); 
#endif

#ifndef _USE_MULTI_MPMUTEX 
  mpmutex_lock(s_etask->csem);
#else
  mpmutex_lock(&s_etask->sem);
#endif
  pr_debug(2,"buf:%p \n",buf); 

  /* initialize a shared memory */
  for(m=0; m<sz; m++){
    buf[m] = 0x00; 
  }
#ifndef _USE_MULTI_MPMUTEX 
  mpmutex_unlock(s_etask->csem);
#else
  mpmutex_unlock(&s_etask->sem);
#endif
#ifdef _USE_DETACH1
  pr_debug(3,"before detach\n"); 
   
  mpshm_detach(&s_etask->shm);  

  pr_debug(3,"after detach\n"); 
#endif
  return;
}

void pr_dbg_workerinfo(sec_etask_t *s_etask, int i, int sz, int offset)
{
  char *buf;
  int m;

  pr_debug(3,"Memory %d\n", i);

#ifdef _USE_ATTACH2
  buf = (char *)mpshm_attach(&s_etask->shm, 0);
  if(buf==NULL){
    err("Attach error!! \n"); 
    return;
  }
#else
  buf = (char *)(s_etask->buf); 
  pr_debug(2,"init buf:%p \n",buf); 
#endif

#ifndef _USE_MULTI_MPMUTEX 
  mpmutex_lock(s_etask->csem);
#else
  mpmutex_lock(&s_etask->sem);
#endif
  for(m=offset; m<sz; m++){
    message("%02x ",buf[m]);
 
    if(!((m+1)%16)) message("\n");
  }
#ifndef _USE_MULTI_MPMUTEX 
  mpmutex_unlock(s_etask->csem);
#else
  mpmutex_unlock(&s_etask->sem);
#endif
#ifdef _USE_DETACH2
   
  mpshm_detach(&s_etask->shm);  

#endif

  pr_debug(3,"Display %d finished \n", i); 

  return;
}
#endif /* DEBUG_ON */
/****************************************************************************
 * Public Functions
 ****************************************************************************/

static int init_param_secure(sec_info_t *s_data)
{
  int i;

  s_data->fullpath[0] = '\0';
#ifdef _2ND_SECGRP_ENA
  s_data->fullpath2[0] = '\0';
#endif 
  s_data->cpucnt=0;
 
  for(i=0; i<SEC_MAXTASKS; i++)
    {
      s_data->s_etask[i].cpuno = i; 
      s_data->s_etask[i].buf = NULL;
      s_data->s_etask[i].shm_allc = SHM_NOALLC;
      s_data->s_etask2[i].cpuno = i; 
      s_data->s_etask2[i].buf = NULL; 
    } 
  return 0;
}
/****************************************************************************
 * Name: sec_init_tdata 
 ****************************************************************************/
static void sec_init_tdata(sec_info_t *s_data)
{ 
  int i;
  sec_task_t *s_task;

//  pr_debug(2,"## MSG_STRING:%s \n",MSG_STRING);
   
  for(i=0; i<MAXTASKS; i++)
    {
      s_task = &(s_data->s_task[i]); 
      strncpy((s_task->tdata).msg, (const char *)MSG_STRING, sizeof(MSG_STRING));
      pr_debug(2,"## tdata:%s  \n",(s_task->tdata).msg); 
    } 
  return;
}
/****************************************************************************
 * Public Functions
 ****************************************************************************/

static int init_param(sec_info_t *s_data)
{
  int i;

//#ifdef _3RD_SECGRP_ENA
  s_data->fullpath3[0] = '\0';

  for(i=0; i<MAXTASKS; i++)
    {
      s_data->s_task[i].taskno = i; 
      s_data->s_task[i].buf = NULL; 
    } 
  sec_init_tdata(s_data);
//#endif  

  return 0;
}
/****************************************************************************
 * set_area_addr 
 ****************************************************************************/
static int set_area_addr(int argc, char *argv[], sec_info_t *s_data)
{
  long int input;

  if(argc >1)
    {
      /* get a parameter to decide a target area */ 
      message("argv[1]:%s \n",argv[0], argv[1]);

      input = strtol(argv[1], NULL, 16);
      pr_debug(2,"target area :%x \n",input);

      if((input >= 0) && (input <= TILE_OFFSET_END))
        {
          s_data->tile_start = (int)input; /* offset address for start */
          pr_debug(2,"tile_start: %x \n",s_data->tile_start); 
        }
      else
        {
          message("argc:%d but out of range \n",argc); 
          s_data->tile_start = TILE_OFFSET_BASE; /* offset address for start */ 
        }

      if(argc >2)
        {
          input = strtol(argv[2], NULL, 16);
          pr_debug(2,"access size:%x \n",input);
          
          if((input >=0) && (input <=TEST_SIZE_END))
            {
              s_data->ac_size = (int)input;
              pr_debug(2,"ac_size: %x \n",s_data->ac_size); 
            }    
        }
      else
        {
          s_data->ac_size = TILE_OFFSET_SZ;
        }
    }
  else
    {
      message("argc:%d so default tile address is used!  \n",argc); 
      s_data->tile_start = TILE_OFFSET_BASE; /* offset address for start */ 
    }
  return 0; 
}

#ifndef _2ND_SECGRP_ENA
/****************************************************************************
 * setup_task_secure 
 ****************************************************************************/
static int setup_task_secure(const char *filename, sec_info_t *s_data)
{
  int ret;
  sec_combinfo_t *combd;
  int i;
  sec_etask_t *s_etask;

  combd = &s_data->s_comb;

  pr_debug(3, "%s filename:%s \n",__func__, filename);
 
  ret = mptask_init_secure(&combd->task, filename);
  if (ret != 0)
    {
      err("mptask_init_secure() failure. %d\n", ret);
      return ret;
    }
#if 1 
  ret = mptask_assign_cpus(&combd->task, CPUS_NUM);
  if (ret != 0)
    {
      err("mptask_assign_cpus() failure. %d\n", ret);
      return ret;
    }

  ret = mptask_getcpuidset(&combd->task, &(combd->cpuids)); 
  if (ret < 0)
    {
      err("mptask_getcpuidset() failure. %d\n", ret);
      return ret;
    }
  pr_debug(4, "cpuids:%p cpuids:%d \n" ,combd->cpuids, combd->cpuids); 
#else
  ret = mptask_assign(&combd->task);
  if (ret != 0)
    {
      err("mptask_assign() failure. %d\n", ret);
      return ret;
    }

#endif
#if 0 
  for(i=0; i<SEC_MAXTASKS; i++)
    { 
      s_etask = &s_data->s_etask[i];
      s_etask->cpuno = i+CPUNO_OFFSET;

      ret = mpmq_init(&s_etask->mq, KEY_MQ, mptask_getcpuid(&combd->task));
      if(ret < 0)
        {
          err("mpmq_init() failure. ret:%d cpuno:%d \n", ret, s_etask->cpuno);
          return ret;
        }
      pr_debug(3,"mpmq_init mq ok at cpu%d\n",mptask_getcpuid(&combd->task) );
       /* Initialize Shared Memory for each MPtask */
        ret = mpshm_init(&s_etask->shm, KEY_SHM, SEC_SZ_SHM);
        if (ret < 0)
          {
            err("mpshm_init() failure. %d\n", ret);
            return ret;
          }
        pr_debug(3,"mpshm init ok \n");
        pr_debug(3,"s_etask->paddr:%p \n",s_etask->shm.paddr);

        s_etask->dtype.paddr = s_etask->shm.paddr;
        s_etask->dtype.size  = SEC_SZ_SHM;

        s_etask->buf = (char *)mpshm_attach(&s_etask->shm, 0);
        if(s_etask->buf==NULL)
          {
            err("mpshm_attach error!! \n"); 
            return ret;
          } 
        pr_dbg_initmem_sec(s_etask, SEC_SZ_SHM);
    }
#else
  for(i=0; i<SEC_MAXTASKS; i++){ 
    s_etask = &s_data->s_etask[i];

    /* Initialize MP message queue with asigned CPU ID, and bind it to MP task */

    if(CPU_ISSET(i+CPUNO_OFFSET, &combd->cpuids))
      {
        s_etask->cpuno = i+CPUNO_OFFSET;
        pr_debug(3, "cpuno:%d \n", s_etask->cpuno); 
        ret = mpmq_init(&s_etask->mq, KEY_MQ, s_etask->cpuno);
        if(ret < 0)
          {
            err("mpmq_init() failure. ret:%d cpuno:%d \n", ret, s_etask->cpuno);
            return ret;
          }
        pr_debug(3,"mpmq_init mq ok \n");
        pr_debug(2,"mq:%p super:%p cpuid:%d flags:%d \n",
             s_etask->mq, s_etask->mq.super, s_etask->mq.cpuid, s_etask->mq.flags);
    
        /* Initialize Shared Memory for each MPtask */
        ret = mpshm_init(&s_etask->shm, KEY_SHM, SEC_SZ_SHM);
        if (ret < 0)
          {
            err("mpshm_init() failure. %d\n", ret);
            return ret;
          }
        pr_debug(3,"mpshm init ok \n");
        pr_debug(3,"s_etask->paddr:%p \n",s_etask->shm.paddr);
 
        s_etask->dtype.paddr = s_etask->shm.paddr;
        s_etask->dtype.size  = SEC_SZ_SHM;

#if 0 
        *(volatile char *)(s_etask->dtype.paddr) = 0xA5;

        if(((char *)(s_etask->dtype.paddr))[0] == 0xA5)
          {
            pr_debug(3," write OK : paddr[0] :%x \n",((char *)(s_etask->dtype.paddr))[0]); 
          }
        else
          {
            pr_debug(3, "physical address write NG \n");
          } 
#endif
        s_etask->buf = (char *)mpshm_attach(&s_etask->shm, 0);
        if(s_etask->buf==NULL)
          {
            err("mpshm_attach error!! \n"); 
            return ret;
          } 
        pr_dbg_initmem_sec(s_etask, SEC_SZ_SHM);
      }
   }
#endif /* using ids */

  return 0;
}
#endif

/****************************************************************************
 * setup_task_secure 
 ****************************************************************************/
static int setup_task_secure3(const char *filename, sec_info_t *s_data, int no)
{
  int ret;
  sec_combinfo_t *combd;
  int i;
  sec_etask_t *s_etask;
  int sz;
  int curnum;
  char * combuf;
  mpshm_t tshm;

  if(no == 1) 
    {
      combd = &s_data->s_comb;
      sz  = CPUS_NUM;
    } 
  else if(no == 2)
    {
      combd = &s_data->s_comb2;
      sz  = CPUS_NUM2;
    }
  else return NG;

  pr_debug(3, "%s filename:%s \n",__func__, filename);
 
  ret = mptask_init_secure(&combd->task, filename);
  if (ret != 0)
    {
      err("mptask_init_secure() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_assign_cpus(&combd->task, sz);
  if (ret != 0)
    {
      err("mptask_assign_cpus() failure. %d\n", ret);
      return ret;
    }

  ret = mptask_getcpuidset(&combd->task, &(combd->cpuids)); 
  if (ret < 0)
    {
      err("mptask_getcpuidset() failure. %d\n", ret);
      return ret;
    }
  pr_debug(4, "GRP[%d] cpuids:%p cpuids:%d \n" ,no, combd->cpuids, combd->cpuids); 

  curnum = s_data->cpucnt;
  switch(no){
  case 1:
  case 2:
    s_data->grp[no-1] = curnum; 
    break;
  default:
    return -EINVAL;
  }   
  for(i=0; i<sz; i++){ 
    s_etask = &s_data->s_etask[curnum +i];
    
    /* Initialize MP message queue with asigned CPU ID, and bind it to MP task */

    if(CPU_ISSET(i +curnum +CPUNO_OFFSET, &combd->cpuids))
      {
        
#ifdef _ENA_SHM
        if(i==0)
          {
            /* 
               allocate only one shm to save total resources 
               Initialize Shared Memory for each MPtask 
             */
            ret = mpshm_init(&s_etask->shm, KEY_SHM, SEC_SZ_SHM);
            if (ret < 0)
              {
                err("mpshm_init() failure. %d\n", ret);
                return ret;
              }
            pr_debug(3,"mpshm init ok \n");
            pr_debug(2, "a first core in GRP etask:%p shm:%p \n",s_etask, s_etask->shm);
            pr_debug(3,"s_etask->paddr:%p \n",s_etask->shm.paddr);
 
            s_etask->dtype.paddr = s_etask->shm.paddr;
            s_etask->dtype.size  = SEC_SZ_SHM;
            s_etask->shm_allc    = SHM_SETALLC;

	    tshm.super = s_etask->shm.super;
	    tshm.tag = s_etask->shm.tag;
	    tshm.paddr = s_etask->shm.paddr;
	    tshm.size = s_etask->shm.size;
	    tshm.exc = s_etask->shm.exc;
 
            pr_debug(3,"shm.super:%x \n",s_etask->shm.super); 
            pr_debug(3,"shm.tag:%x \n",s_etask->shm.tag); 
            pr_debug(3,"shm.paddr:%x \n",s_etask->shm.paddr); 
            pr_debug(3,"shm.size:%x \n",s_etask->shm.size); 
            pr_debug(3,"shm.exc:%x \n",s_etask->shm.exc); 
            s_etask->buf = (char *)mpshm_attach(&s_etask->shm, 0);
            if(s_etask->buf==NULL)
              {
                err("mpshm_attach error!! \n"); 
                return ret;
              }
            combuf = s_etask->buf; 
            pr_dbg_initmem_sec(s_etask, SEC_SZ_SHM);
          }
        else
          {
            s_etask->shm_allc    = SHM_NOALLC;

	    s_etask->shm.super = tshm.super;
	    s_etask->shm.tag = tshm.tag;
	    s_etask->shm.paddr = tshm.paddr;
	    s_etask->shm.size = tshm.size;
	    s_etask->shm.exc = tshm.exc;
            s_etask->dtype.paddr = s_etask->shm.paddr;
            s_etask->dtype.size  = SEC_SZ_SHM;
            s_etask->buf = combuf;

            pr_debug(3,"tshm.super:%x \n",tshm.super); 
            pr_debug(3,"tshm.tag:%x \n",tshm.tag); 
            pr_debug(3,"tshm.paddr:%x \n",tshm.paddr); 
            pr_debug(3,"tshm.size:%x \n",tshm.size); 
            pr_debug(3,"tshm.exc:%x \n",tshm.exc); 
            if(s_etask->buf==NULL)
              {
                err("buf reference error!! at %d \n",i); 
                return ret;
              } 
          }
#endif
        s_etask->cpuno = i +curnum +CPUNO_OFFSET;
        s_data->cpucnt++;
        pr_debug(3, "cpuno:%d cpucnt:%d \n", s_etask->cpuno, s_data->cpucnt); 
        ret = mpmq_init(&s_etask->mq, KEY_MQ, s_etask->cpuno);
        if(ret < 0)
          {
            err("mpmq_init() failure. ret:%d cpuno:%d \n", ret, s_etask->cpuno);
            return ret;
          }
        pr_debug(3,"mpmq_init mq ok \n");
        pr_debug(2,"mq:%p super:%p cpuid:%d flags:%d \n",
             s_etask->mq, s_etask->mq.super, s_etask->mq.cpuid, s_etask->mq.flags);
    
      }
   }
  return 0;
}

/****************************************************************************
 * setup_task 
 ****************************************************************************/
static int setup_task(const char *filename, sec_task_t *s_task)
{
  int ret;
  void *virtaddr=NULL;

  pr_debug(2,"==> %s \n",__func__);
 
  /* Initialize MP task */

  ret = mptask_init(&s_task->task, filename);
    if (ret != 0)
      {
        err("mptask_init() failure. %d\n", ret);
        return ret;
      }
  ret = mptask_assign(&s_task->task);
  if (ret != 0)
    {
      err("mptask_asign() failure. %d\n", ret);
      return ret;
    }
  message("assigned at CPU%d\n", mptask_getcpuid(&s_task->task));

#ifdef _USE_MULTI_MPMUTEX 
  /* Initialize MP mutex and bind it to MP task */

  ret = mpmutex_init(&s_task->sem, KEY_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }
  ret = mptask_bindobj(&s_task->task, &s_task->sem);
#else
  /* bind it to MP task */
  ret = mptask_bindobj(&s_task->task, s_task->csem);
#endif
  
  if (ret < 0)
    {
      err("mptask_bindobj(mutex) failure. %d l:%d \n", ret, __LINE__);
      return ret;
    }
  pr_debug(3, "mptask_bind sem ok \n");

  /* Initialize MP message queue with asigned CPU ID, and bind it to MP task */

  ret = mpmq_init(&s_task->mq, KEY_MQ, mptask_getcpuid(&s_task->task));
  if (ret < 0)
    {
      err("mpmq_init() failure. %d\n", ret);
      return ret;
    }
  pr_debug(3,"mpmq_init mq ok \n");
  pr_debug(3,"mq:%p super:%p cpuid:%d flags:%d \n",s_task->mq, s_task->mq.super, s_task->mq.cpuid, s_task->mq.flags);

  ret = mptask_bindobj(&s_task->task, &s_task->mq);
  if (ret < 0)
    {
      err("mptask_bindobj(mq) failure. %d l:%d \n", ret, __LINE__);
      return ret;
    }
  pr_debug(3,"mptask_bind mq ok \n");

  /* Initialize Shared Memory for each MPtask */
  ret = mpshm_init(&s_task->shm, KEY_SHM, SEC_SZ_SHM);
  if (ret < 0)
    {
      err("mpshm_init() failure. %d\n", ret);
      return ret;
    }
  pr_debug(3,"mpshm->paddr:%p \n",s_task->shm.paddr);
 
  s_task->buf = (char *)mpshm_attach(&s_task->shm, 0);
  if(s_task->buf==NULL){
    err("mpshm_attach error!! \n"); 
    return ret;
  } 
  virtaddr = mpshm_phys2virt(&s_task->shm, s_task->shm.paddr); 
  pr_debug(2,"virt address : %p \n",virtaddr); 

  ret = mptask_bindobj(&s_task->task, &s_task->shm);
  if (ret < 0)
    {
      err("mptask_bindobj(shm) failure. %d l:%d \n", ret, __LINE__);
      return ret;
    }
#ifdef DEBUG_ON
  pr_dbg_initmem(s_task, SEC_SZ_SHM);
  pr_debug(3,"shm init finished \n");
#endif

  return 0;
}

/****************************************************************************
 * finalize task 
 ****************************************************************************/
static int finalize_task_secure3(sec_info_t *s_data)
{
  int ret;
  int wret;
  int i;
  sec_combinfo_t *s_comb; 
  sec_etask_t *etask;

  s_comb = &s_data->s_comb;

  pr_debug(3,"==> %s \n",__func__); 

#ifdef _ENA_SHM
  for (i=0; i<s_data->cpucnt; i++){

    etask = &(s_data->s_etask[i]);
    mpmq_destroy(&etask->mq);
    pr_debug(4, "No.%d etask:%p mq:%p \n",i,etask, etask->mq);
  
    if(etask->shm_allc == SHM_SETALLC)
    { 
      pr_debug(4, "shm:%p \n", etask->shm);
      mpshm_detach(&etask->shm);  
      mpshm_destroy(&etask->shm);
    }
  }  
#endif

  /* Finalize MP task */

  wret = NG;
  /* true means forcibly shutdown */
  ret = mptask_destroy(&s_comb->task, true, &wret);
  if (ret < 0)
    {
      err("mptask_destroy() group 1 failure. %d\n", ret);
      return ret;
    }

  message("grp1 worker exit status = %d\n", wret);
  
  s_comb = &s_data->s_comb2;
  wret = NG;
  ret = mptask_destroy(&s_comb->task, true, &wret);
  if (ret < 0)
    {
      err("mptask_destroy() group 2 failure. %d\n", ret);
      return ret;
    }

  message("grp2 worker exit status = %d\n", wret);
  
  pr_debug(3,"<== %s \n",__func__); 
  return 0;
}

/****************************************************************************
 * finalize task 
 ****************************************************************************/
static int finalize_task(sec_info_t *s_data)
{
  int ret;
  int wret;
  int i;
  sec_task_t *task;

  pr_debug(3,"==> %s \n",__func__); 

  /* Finalize MP task */
  i=0;
    task = &(s_data->s_task[i]);
 
    wret = NG;

    /* true means forcibly shutdown */
    ret = mptask_destroy(&(task->task), true, &wret);
    if (ret < 0)
      {
        err("mptask_destroy() failure. %d\n", ret);
        return ret;
      }

    message("Worker exit status = %d\n", wret);

    mpmq_destroy(&task->mq);
    mpshm_detach(&task->shm);  
    mpshm_destroy(&task->shm);
  mpmutex_destroy(&(s_data->sem));

  pr_debug(3,"<== %s \n",__func__); 
  return 0;

}

#ifndef _3RD_SECGRP_ENA
/****************************************************************************
 * start_task_secure 
 ****************************************************************************/
static int start_task_secure(sec_info_t *s_data)
{
  int ret;
  sec_combinfo_t *s_comb;

  s_comb = &s_data->s_comb;

  /* Run worker */

  pr_debug(3,"task:%p \n",&s_comb->task);
  pr_debug(3,"comb:%p \n",s_comb);
  pr_debug(3,"s_etask.task:%p \n",&s_data->s_comb.task);
  pr_debug(3,"s_etask:%p \n",&s_data->s_comb.task);

  ret = mptask_exec(&s_comb->task);
  if (ret != 0)
    {
      err("mptask_exec() failure. %d\n", ret);
      return ret;
    }
  else
    {
      pr_debug(3,"mptask_exec OK ret:%d\n",ret);
    }

  return 0;

}
#endif
/****************************************************************************
 * start_task_secure2 
 ****************************************************************************/
static int start_task_secure2(sec_info_t *s_data, int no)
{
  int ret;
  sec_combinfo_t *s_comb;

  pr_debug(2, "==> %s \n", __func__);
 
  if(no==1) s_comb = &s_data->s_comb;
  else if(no==2) s_comb = &s_data->s_comb2;
  else return NG;

  /* Run worker */

  pr_debug(3,"task:%p \n",&s_comb->task);
  pr_debug(3,"comb:%p \n",s_comb);
  pr_debug(3,"s_etask.task:%p \n",&s_data->s_comb.task);
  pr_debug(3,"s_etask:%p \n",&s_data->s_comb.task);

  ret = mptask_exec(&s_comb->task);
  if (ret != 0)
    {
      err("mptask_exec() failure. %d\n", ret);
      return ret;
    }
  else
    {
      pr_debug(3,"mptask_exec OK ret:%d\n",ret);
    }

  return 0;
}

/****************************************************************************
 * send_fistmsg_secure 
 ****************************************************************************/
static int send_firstmsg_secure(sec_info_t *s_data)
{
  int i;
  int ret;
  sec_etask_t *s_etask;
  sec_mq_dtype_t *mqdtype_d;
//  uintptr_t t_paddr;

  pr_debug(2,"==> %s \n",__func__); 

//  for (i=0; i<SEC_MAXTASKS; i++)
  for (i=0; i<s_data->cpucnt; i++)
    {

      /* Send command to worker */
      s_etask = &(s_data->s_etask[i]);
#ifdef DEBUG_ON
//       pr_dbg_workerinfo(s_etask, 1, PR_MEM_SZ, 0);
#endif
 
      mqdtype_d = (sec_mq_dtype_t *)(s_etask->buf);
      mqdtype_d->paddr = s_etask->dtype.paddr;
      pr_debug(3,"mqdtype_d->paddr:%x \n",mqdtype_d->paddr);

#if 0 
      t_paddr = (uint32_t)mpshm_virt2phys(&s_etask->shm, (void *)&(mqdtype_d->data));
      pr_debug(3,"t_paddr p:%p \n",t_paddr);
#endif

      up_udelay(1000000); 
      strncpy((char *)&(mqdtype_d->data), (const char *)MSG_STRING, sizeof(MSG_STRING));

      pr_debug(3,"Message:%s \n",(char *)&(mqdtype_d->data));
      
      pr_dbg_workerinfo2(mqdtype_d, 0x80, 0);
      ret = mpmq_send(&s_etask->mq, MSG_ID_SAYHELLO, mqdtype_d->paddr);
      if (ret < 0)
        {
          err("mpmq_send(task%d) failure %d\n", i, ret);
          return ret;
        }
      else
        {
          pr_debug(3,"mpmq_send(task%d) OK \n",i);
        }
    }
  pr_debug(2,"<== %s \n",__func__); 

  return 0;
}

/****************************************************************************
 * start_task 
 ****************************************************************************/
static int start_task(sec_task_t *s_task)
{
  int ret;

  /* Run worker */

  ret = mptask_exec(&s_task->task);
  if (ret < 0)
    {
      err("mptask_exec() failure. %d\n", ret);
      return ret;
    }

  return 0;

}

/****************************************************************************
 * send_fistmsg 
 ****************************************************************************/
static int send_firstmsg(sec_info_t *s_data)
{
  int i;
  int ret;
  sec_task_t *s_task;

  pr_debug(2,"==> %s \n",__func__); 

//  for (i=0; i<SEC_MAXTASKS; i++)
//  for (i=0; i<1; i++)
    i=0;
    {

      /* Send command to worker */
      s_task = &(s_data->s_task[i]);
#ifdef DEBUG_ON
       pr_dbg_workerinfo(s_task, 1, PR_MEM_SZ, 0);
#endif
        
      ret = mpmq_send(&s_task->mq, MSG_ID_SAYHELLO, 0xdeadbeef);
      if (ret < 0)
        {
          err("mpmq_send(task%d) failure %d\n", i, ret);
          return ret;
        }
      else
        {
          pr_debug(3,"mpmq_send(task%d) OK \n",i);
        }
    }
  pr_debug(2,"<== %s \n",__func__); 

  return 0;
}
#ifdef _TEST_NONSECURE

/****************************************************************************
 * send_fistmsg2 
 ****************************************************************************/
static int send_firstmsg2(sec_info_t *s_data)
{
  int i;
  int ret;
  sec_task_t *s_etask;
  sec_combinfo_t *combd; 
  mptask_t task; 
  combd = &s_data->s_etask;
 
  for (i=0; i<SEC_MAXTASKS; i++)
    {

      /* Send command to worker */
      s_etask = &(s_data->s_etask[i]);
#ifdef DEBUG_ON
       pr_dbg_workerinfo(s_etask, 1, PR_MEM_SZ, 0);
#endif
      ret = mpmq_send(&s_etask->mq, MSG_ID_SAYHELLO, );
      if (ret < 0)
        {
          err("mpmq_send(task%d) failure %d\n", i, ret);
          return ret;
        }
      else
        {
          pr_debug(3,"mpmq_send(task%d) OK \n",i);
        }
    }

  return 0;
}
#endif
/****************************************************************************
 * set_rootfs 
 ****************************************************************************/
static int set_rootfs(sec_info_t *s_data)
{

#ifdef CONFIG_FS_ROMFS
  int ret;
  struct stat buf;

  ret = stat(MOUNTPT, &buf);
  if (ret < 0)
    {
      message("Registering romdisk at /dev/ram0\n");
      ret = romdisk_register(0, (FAR uint8_t *)romfs_img,
                             NSECTORS(romfs_img_len), SECTORSIZE);
      if (ret < 0)
        {
          err("ERROR: romdisk_register failed: %d\n", ret);
          exit(1);
        }

      message("Mounting ROMFS filesystem at target=%s with source=%s\n",
              MOUNTPT, "/dev/ram0");

      ret = mount("/dev/ram0", MOUNTPT, "romfs", MS_RDONLY, NULL);
      if (ret < 0)
        {
          err("ERROR: mount(%s,%s,romfs) failed: %s\n",
              "/dev/ram0", MOUNTPT, errno);
          exit(1);
        }
    }
#endif

#ifdef CONFIG_FS_ROMFS
    snprintf(s_data->fullpath, 128, "%s/%s", MOUNTPT, "sectest");
#else
    snprintf(s_data->fullpath, 128, "%s", "sectest");
#ifdef _2ND_SECGRP_ENA
    snprintf(s_data->fullpath2, 128, "%s", "sectest2");
#endif
#ifdef _3RD_SECGRP_ENA
    snprintf(s_data->fullpath3, 128, "%s%s", "/mnt/sd0/","sectest3");
#endif
#endif

  pr_debug(3, "%s : process OK \n",__func__); 
  return 0;

}
#ifdef _USE_SIGNAL 
/****************************************************************************
 * init_for_signal
 ****************************************************************************/
static int init_for_signal(sec_info_t *s_data)
{
  int i;
  sigset_t *sig;
  sigset_t *tsig;
  int sigdata;
  int ret;

  sig = &(s_data->sig_d); 
  tsig = &(s_data->th_sig_d); 
  
  pr_debug(3, "%s pid:%x \n",__func__, getpid());
  s_data->my_pid = getpid();
 
  sigemptyset(sig);
  sigemptyset(tsig);

  sigaddset(tsig, THREAD_SIG_NO); /*  for pthread */
  ret = sigprocmask(SIG_UNBLOCK, tsig, NULL);
  if(ret) return NG;

  for (i=0; i<SEC_MAXTASKS; i++)
    { 
      sigaddset(sig, sig_type[i]);
    }
  ret = sigprocmask(SIG_BLOCK, sig, NULL);
  if(ret) return NG;
  pr_debug(3, "mask sig:%x \n",sig); 

  for (i=0; i<SEC_MAXTASKS; i++)
    { 
      sigaddset(sig, sig_type[i]);
      ret = sigprocmask(SIG_BLOCK, sig, NULL);
      if(ret) return NG;
      pr_debug(3, "mask sig:%x \n",sig); 

     /* set a sig_no on each of the mpmq */
      sigdata = true; /* Note: the type of argument to pass this value is union */ 
      mpmq_notify(&(s_data->s_etask[i].mq), sig_type[i], (void *)sigdata);
    }
    sigdata = true; /* Note: the type of argument to pass this value is union */ 
    mpmq_notify(&(s_data->s_etask[0].mq), THREAD_SIG_NO, (void *)sigdata);
#if 0
  for (i=0; i<SEC_MAXTASKS; i++)
    {
      ret = sigprocmask(SIG_UNBLOCK, sig, NULL);
      if(ret) return NG;
    }
#endif
  return 0; 

}
#endif
/****************************************************************************
 * init_for_signal2
 ****************************************************************************/
static int init_for_signal2(sec_info_t *s_data)
{
  sigset_t *tsig;
  int ret;

  tsig = &(s_data->th_sig_d); 
  
  pr_debug(3, "%s pid:%x \n",__func__, getpid());
  s_data->my_pid = getpid();
 
  sigemptyset(tsig);

  sigaddset(tsig, THREAD_SIG_NO);     /*  for pthread */
  sigaddset(tsig, THREAD_SIG_NO_ERR); /*  for pthread(err) */
  ret = sigprocmask(SIG_UNBLOCK, tsig, NULL);
  if(ret) return NG;

  return 0; 
}

#ifdef _USE_SIGNAL 
/****************************************************************************
 * unmask_signal
 ****************************************************************************/
static int unmask_signal(sec_info_t *s_data)
{
  int i;
  sigset_t *sig;
  int ret;

  for (i=0; i<SEC_MAXTASKS; i++)
    {
      sig = &(s_data->sig_d); 
      pr_debug(3, "unmask sig:%x \n",sig); 
      ret = sigprocmask(SIG_UNBLOCK, sig, NULL);
      if(ret) return NG;
    }
  return 0;  
}
#endif

#ifdef _PROT_TEST 
/****************************************************************************
 * Name: sec_param_worker
 ****************************************************************************/
static int set_param_worker_secure(sec_info_t *sec_data)
{
  int i=0;
  int ret;
#ifdef _3RD_SECGRP_ENA
//  int cpucnt=0;
#endif

  /* Initialize a common MP mutex for all tasks */
  ret = mpmutex_init(&sec_data->sem, KEY_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }

#ifdef _2ND_SECGRP_ENA
  /* final TEST target */

  /* setup a task for secure group1 */
  ret = setup_task_secure2(sec_data->fullpath, sec_data, 1);
  if(ret)
    {
       goto test_fail;
    } 
  /* setup a task for secure group2 */
  ret = setup_task_secure2(sec_data->fullpath2, sec_data, 2);
  if(ret)
    {
      goto test_fail;
    }
#ifdef _3RD_SECGRP_ENA
//    cpucnt = sec_data->cpucnt;
    sec_data->s_task[0].csem = &(sec_data->sem);
  
    /* setup a task for non-secure */
    ret = setup_task(sec_data->fullpath3, &(sec_data->s_task[0]));
    if(ret)
      {
        goto test_fail;
      }
//    sec_data->cpucnt++;
     pr_dbg_initmem(&(sec_data->s_task[0]), SEC_SZ_SHM);
#endif
#else
  ret = setup_task_secure(sec_data->fullpath, sec_data);
  if(ret)
    {
       goto test_fail;
    }  
#endif

#ifdef _2ND_SECGRP_ENA
  for(i=0; i<2; i++){
    ret = start_task_secure2(sec_data, i+1);
    if(ret)
      {
        goto test_fail;
      }
    else
      {
        pr_debug(3, "group%d started \n",i+1); 
      }
  }
#ifdef _3RD_SECGRP_ENA
//  cpucnt = sec_data->cpucnt -1;
  ret = start_task(&(sec_data->s_task[0]));
  if(ret)
    {
      goto test_fail;
    }
  else
    {
      message("non secure task started \n");
    }
#endif
#else
  ret = start_task_secure(sec_data);
  if(ret)
    {
      goto test_fail;
    }
  else
    {
      pr_debug(3, "group1 started \n"); 
    }
#endif
  return 0;
test_fail:
  return NG;
}
#endif

#ifdef _PROT_TEST 
/****************************************************************************
 * Name: sec_param_worker
 ****************************************************************************/
static int set_param_worker_secure3(sec_info_t *sec_data)
{
  int i=0;
  int ret;
#ifdef _3RD_SECGRP_ENA
//  int cpucnt=0;
#endif

  /* Initialize a common MP mutex for all tasks */
  ret = mpmutex_init(&sec_data->sem, KEY_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }

  /* final TEST target */

  /* setup a task for secure group1 */
  ret = setup_task_secure3(sec_data->fullpath, sec_data, 1);
  if(ret)
    {
       goto test_fail;
    } 
  /* setup a task for secure group2 */
  ret = setup_task_secure3(sec_data->fullpath2, sec_data, 2);
  if(ret)
    {
      goto test_fail;
    }
//    cpucnt = sec_data->cpucnt;
#ifdef _3RD_SECGRP_ENA
    sec_data->s_task[0].csem = &(sec_data->sem);

  
    /* setup a task for non-secure */
    ret = setup_task(sec_data->fullpath3, &(sec_data->s_task[0]));
    if(ret)
      {
        goto test_fail;
      }
//    sec_data->cpucnt++;
     pr_dbg_initmem(&(sec_data->s_task[0]), SEC_SZ_SHM);
#endif

  for(i=0; i<2; i++){
    ret = start_task_secure2(sec_data, i+1);
    if(ret)
      {
        goto test_fail;
      }
    else
      {
        pr_debug(3, "group%d started \n",i+1); 
      }
  }
//  cpucnt = sec_data->cpucnt -1;
#ifdef _3RD_SECGRP_ENA
  ret = start_task(&(sec_data->s_task[0]));
  if(ret)
    {
      goto test_fail;
    }
  else
    {
      message("non secure task started \n");
    }
#endif
  return 0;
test_fail:
  return NG;
}
#endif

#ifdef _USE_WAITSIG
/****************************************************************************
 * Name: pr_err_task 
 ****************************************************************************/
static int pr_err_task(siginfo_t *sinfo)
{
  int data;
  int i;
 
  data = sinfo->si_value.sival_int;

  for(i=0; i<SEC_MAXTASKS; i++)
    {
      if((data >> i)&1)
        {
          pr_debug(2, " %s data[d]:%d cnt:%d \n",__func__, data, i);
          return i; 
        } 
    }
  return NG; 
} 
#endif
#ifdef _USE_WAITSIG
/****************************************************************************
 * Name: wait_signal 
 ****************************************************************************/
static int wait_signal(sec_info_t *s_data)
{
  int ret;
  sigset_t *sig;
  siginfo_t sinfo;
  struct timespec timeout;

  timeout.tv_sec = SIG_INTERVAL_WAIT;

  sig = &(s_data->sig_d); 

/*----------------------------------------------------------------*/
  while(1){ 
 
    /* set a few mask for signal */ 
    sigemptyset(sig);
    sigaddset(sig, THREAD_SIG_NO);
    sigaddset(sig, THREAD_SIG_NO_ERR);

    pr_debug(3, "wait sig:%x \n",*sig);

    /* wait for a signal from all tasks */
    ret = sigtimedwait(sig, &sinfo, &timeout);
    if(ret>0)
      {
        switch (ret)
          {
            case THREAD_SIG_NO:
              message("Test Succeeded; code:%d val:%d \n",
                      sinfo.si_code, sinfo.si_value.sival_int);
              break;
            case THREAD_SIG_NO_ERR:
              ret = pr_err_task(&sinfo); 
              if(ret >= 0)
                {
                  /* success */
                  message("Test Failed; code:%d val:%d at task%d \n",
                          sinfo.si_code, sinfo.si_value.sival_int, ret);
                  break;
                }
              else
                {
                  /* error  */
                  err("Test Failed but no task number: code:%d val:%d \n",
                       sinfo.si_code, sinfo.si_value.sival_int);   
                  return NG;
                }   
              break;
            default:
              err("error not supported the number of task!:%d \n",ret);
          }
          pr_debug(1, "signal received now break! \n");
          break;
      }
    else
     {
       err("Time out at waiting a signal\n"); 
       return NG;
     }
  }
  return 0;
}
#endif

#ifdef _USE_WAIT_FIRSTMSG
/****************************************************************************
 * Name: wait_firstmsg 
 ****************************************************************************/
static int wait_firstmsg(sec_info_t *s_data)
{
  int ret;
  sigset_t *sig;
  sec_task_t *s_etask;
  void *vaddr;
#ifndef _TEST_MQ_RECEIVE
  int i;
  siginfo_t sinfo;
  int count=0;
  struct timespec timeout;
#else
  uint32_t msgdata;
#endif
  union sigval value;

  timeout.tv_sec = SIG_INTERVAL_WAIT;

  sig = &(s_data->sig_d); 

  pr_debug(3,"wait sig:%x \n",*sig);

#ifdef _TEST_MQ_RECEIVE
/*----------------------------------------------------------------*/
  s_etask = &(s_data->s_etask[0]);

  ret = mpmq_receive(&s_etask->mq, &msgdata);
  if (ret < 0)
    {
      err("mpmq_recieve() failure. %d\n", ret);
      return ret;
    }
  pr_debug(3,"wait receive addr:%p \n", msgdata);
  vaddr  = mpshm_phys2virt(&s_etask->shm, (uintptr_t)msgdata);
  if(vaddr == NULL) return -1;  
  pr_debug(3,"vaddr:%p \n", vaddr);

  pr_debug(3,"mq:%p super:%p cpuid:%d flags:%d \n",s_etask->mq, s_etask->mq.super, s_etask->mq.cpuid, s_etask->mq.flags);


#else
/*----------------------------------------------------------------*/
  for(i=0; i<SEC_MAXTASKS; i++){ 
    s_etask = &(s_data->s_etask[i]);
#ifdef DEBUG_ON
    pr_dbg_workerinfo(s_etask, 3, PR_MEM_SZ, 0);
#endif
    sigemptyset(sig);
    sigaddset(sig, THREAD_SIG_NO);

    /* wait for a signal from all tasks */
    ret = sigtimedwait(sig, &sinfo, &timeout);
    if(ret>0)
      {
#ifdef DEBUG_ON
        pr_dbg_workerinfo(s_etask, 4, PR_MEM_SZ, 0);
#endif
#ifndef _USE_MULTI_MPMUTEX 
        mpmutex_lock(s_etask->csem);
#else
        mpmutex_lock(&s_etask->sem);
#endif
        switch (ret)
          {
            case MY_TASK1_SIG:
              message("received a msg from the No.1 task! code:%d val:%d \n",ret, sinfo.si_code, sinfo.si_value.sival_int);
              vaddr  = mpshm_phys2virt(&s_etask->shm, (uintptr_t)(sinfo.si_value.sival_int));
              if(vaddr == NULL) return -1; 
              pr_debug(3,"vaddr:%p Worker said %s \n",vaddr, vaddr); 
              count++;
              break;
            case MY_TASK2_SIG:
              message("received a msg from the No.2 task! code:%d val:%d \n",ret, sinfo.si_code, sinfo.si_value.sival_int);
              count++;
              break;
            case MY_TASK3_SIG:
              message("received a msg from the No.3 task! code:%d val:%d \n",ret, sinfo.si_code, sinfo.si_value.sival_int);
              count++;
              break;
            case MY_TASK4_SIG:
              message("received a msg from the No.4 task! code:%d val:%d \n",ret, sinfo.si_code, sinfo.si_value.sival_int);
              count++;
              break;
            case MY_TASK5_SIG:
              message("received a msg from the No.5 task! code:%d val:%d \n",ret, sinfo.si_code, sinfo.si_value.sival_int);
              count++;
              break;
            default:
              err("error not supported the number of task!:%d \n",ret);
          }
#ifndef _USE_MULTI_MPMUTEX 
        mpmutex_unlock(s_etask->csem);
#else
        mpmutex_unlock(&s_etask->sem);
#endif
      }
    else
     {
       err("error sigtimedwait ret:%d \n",ret);
#ifdef _TEST_SIG
       value.sival_int = true;
       ret = sigqueue(s_data->th_pid, THREAD_SIG_NO, value);
       if(ret<0)
         {
           err("sigqueue error:%x \n",ret); 
           return NG;
         }
       else
         {
           pr_debug(3, "sigqueue sent \n"); 
         } 
#endif       
     }
  }
  if(count >= SEC_MAXTASKS)
    {
      return 0;
    }
  else
    {
      err("error some tasks have Failed! \n");
      return NG;
    }
#endif
  return 0;
}
#endif
#if 0
/****************************************************************************
 * Name: sec_chk_msg 
 ****************************************************************************/
static int sec_chk_msg(uint32_t *msgdata, sec_task_t *s_etask, int no)
{
  int ret = 0;
  int cpuid = 0;
  char *buf = (char *)msgdata; 
  int i;

#ifndef _USE_MULTI_MPMUTEX 
  mpmutex_lock(s_etask->csem);
#else
  mpmutex_lock(&s_etask->sem);
#endif

  cpuid = buf[INDX_CPUID]&MASK_CPUID;

  for(i=0; i<0x10; i++){
    printf("%x ",buf[i]);
 
    if(!((i+1)%16)) printf("\n");
  }
#ifndef _USE_MULTI_MPMUTEX 
  mpmutex_unlock(s_etask->csem);
#else
  mpmutex_unlock(&s_etask->sem);
#endif
  if(cpuid == (no + OFFSET_CPUID))
    {
      message("Worker response: ID = %d, data = %08x from CPUID:%d :OK\n",
          ret, msgdata, cpuid);
      return 0;
    }
  else
   { 
      message("Worker response: ID = %d, data = %08x from CPUID:%d :NG\n",
          ret, msgdata, cpuid);
      return NG; 
   }
}
#endif
#ifdef _USE_THREAD
/****************************************************************************
 * Name: sec_test_monitor_sec 
 ****************************************************************************/
static FAR void sec_test_monitor_sec(FAR void *data)
{
  int i;
  int ret;
//  uint32_t msgdata;
  sec_info_t *s_data;
  sec_etask_t *s_etask;
  int count[SEC_MAXTASKS];
  int exit_flag=0;
#ifndef _ASMP_SEC
  sec_mq_dtype_t *dtype_d;
#endif
 
  pr_debug(2, "<-- pthread start \n");
  
  s_data = (sec_info_t *)data;

  for(i=0; i<SEC_MAXTASKS; i++){
    count[i] = 1;
  } 

  s_data->th_pid = getpid();

  up_udelay(1000000); 
  
  while(exit_flag<SEC_MAXTASKS){ 
//  while(!exit_flag){ 
    for(i=0; i<SEC_MAXTASKS; i++)
      {
        s_etask = &(s_data->s_etask[i]);

#ifndef _ASMP_SEC
        pr_debug(3, "wait for receiving a mq message \n");
        ret = mpmq_receive(&s_etask->mq, &msgdata);
        if (ret < 0)
          {
            err("mpmq_recieve() failure. %d\n", ret);
            goto err;  
          }
        else
          {
            switch (ret){
            case MSG_ID_SAYHELLO:
              message("received MSG_ID_SAYHELLO:%d \n",ret);
              dtype_d = (sec_mq_dtype_t *)msgdata;

              pr_debug(3,"recvd:%p from worker:%p \n",dtype_d->paddr, dtype_d->wpaddr);
              pr_debug(3,"Worker said:%s \n",((char *)(dtype_d->paddr) +0x20));
              exit_flag = 1; 
              break;
            case MSG_ID_TESTMSG:
              message("received MSG_ID_TESTMSG:%d \n",ret);
              /* in case of OK */
              count[i]++;
              break; 
            default:
              err("error not support MSG_ID:%d \n",ret); 
            }
          }
#else
         if(count[i] < TEST1_LOOP_COUNT)
           { 
             if(((char *)s_etask->buf)[0x30] == (0x33 + i))
               { 
                 count[i]++;
               }
           }
         else 
           {
             exit_flag++;
           }
#endif
      }  
  }
  ret = set_signal_asrslt(s_data, s_data->my_pid);
  if(ret) goto err; 

//skip: 
//  pr_debug(2,"--> pthread end\n");
//  return;
err:
  pr_debug(2, "--> pthread error end \n");
  return;
}
#endif

#ifdef _TEST_NONSECURE
/****************************************************************************
 * Name: set_char_formsg
 ****************************************************************************/
int set_char_formsg(char *data, int no, int cnt)
{
  char tmp[2];
  char tmp2[5];

  if (cnt > 0xFF)
    {
       err("Erro parameter INVAL: count over: %d \n", cnt);
       return -EINVAL;
    }

  snprintf(tmp, 2, "%d", no);
  snprintf(tmp2, 5, "%d", cnt);
 
  data[MSG_IND_TASK] = tmp[0];
  data[MSG_IND_CNT1] = tmp2[0];
  data[MSG_IND_CNT1+1]  = '\0';

  return 0;
} 
/****************************************************************************
 * Name: sec_send_nextmsg 
 ****************************************************************************/
static int sec_send_nextmsg(sec_info_t *s_data, int no, int cnt)
{
  int ret;
  uint32_t msgdata;
  uint32_t sndpaddr;
  sec_task_t *s_etask;

  pr_debug(3, "==> %s \n",__func__);

  s_etask = &(s_data->s_etask[no]);
  msgdata = (uint32_t)((char *)(s_etask->buf) + 0x30);
  sndpaddr = (uint32_t)mpshm_virt2phys(&s_etask->shm, (void *)msgdata);

  ret = set_char_formsg(s_etask->tdata.msg, no, cnt);
  if(ret)
    {
      pr_debug(3, "<== %s err1 \n",__func__);
      return ret;
    }
  mpmutex_lock(s_etask->csem);
  strncpy((char *)msgdata, (const char *)(s_etask->tdata.msg), sizeof(s_etask->tdata.msg));
  mpmutex_unlock(s_etask->csem);

  pr_debug(3,"Msg to send from task%d : %s \n",no, msgdata);
  pr_debug(1, "sndpaddr:%p \n",sndpaddr); 
 
  ret = mpmq_send(&s_etask->mq, MSG_ID_TESTMSG, sndpaddr);
  if (ret < 0)
    {
      err("mpmq_send(task%d) failure %d\n", no, ret);
      pr_debug(3, "<== %s err2 \n",__func__);
      return ret;
    }
   else
    {
      pr_debug(3,"mpmq_send(task%d) OK \n",no);
    }

  pr_debug(3, "<== %s ok \n",__func__);
  return 0;
}
#endif

#ifdef _USE_THREAD
/****************************************************************************
 * Name: set_signal_asrslt 
 ****************************************************************************/
static int set_signal_asrslt(sec_info_t *s_data, pid_t pid) 
{
  union sigval value;
  int ret;

  /* setting for signal */
  value.sival_int = true;
  ret = sigqueue(s_data->my_pid, THREAD_SIG_NO, value);
  if(ret<0)
    {
      pr_debug(3, "sigqueue error(at a correct rcvmsg):%x \n",ret);
       return NG;
    }
  else
    {
       pr_debug(3, "sigqueue sent(ok) at a correct process \n");
    }
  return 0;
}
#endif
#ifdef _TEST_NONSECURE
/****************************************************************************
 * Name: set_signal_as_rslt 
 ****************************************************************************/
static int set_signal_as_rslt(sec_info_t *s_data, int no, pid_t pid) 
{
  union sigval value;
  int ret;

  /* setting for signal */
  value.sival_int = true;
  ret = sigqueue(s_data->my_pid, THREAD_SIG_NO, value);
  if(ret<0)
    {
      pr_debug(3, "sigqueue error(at a correct rcvmsg):%x \n",ret);
       return NG;
    }
  else
    {
       pr_debug(3, "sigqueue sent(ok) at a correct process \n");
    }
  return 0;
}
/****************************************************************************
 * Name: sec_signal_err 
 ****************************************************************************/
static int set_signal_err(sec_info_t *s_data, int no, pid_t pid) 
{
  union sigval value;
  int ret;
  int data=0; 
 
  /* setting for signal */
  data = 1<<no;
  value.sival_int = data;
  ret = sigqueue(pid, THREAD_SIG_NO_ERR, value);
  if(ret<0)
    {
       err("sigqueue error(at err):%x \n",ret);
       return ret;
    }
  else
    {
       pr_debug(3,"sigqueue sent(for err) \n");
    }
  return 0; 
}

/****************************************************************************
 * Name: sec_test_monitor5 
 ****************************************************************************/
static FAR void sec_test_monitor5(FAR void *data)
{
  int i;
  int ret;
  uint32_t msgdata;
  sec_info_t *s_data;
  sec_task_t *s_etask;
  int count[SEC_MAXTASKS];
  int exit_flag=0;
  void *vaddr=NULL;
//  sigset_t *sig;
//  siginfo_t sinfo;
  sec_mq_dtype_t *dtype_d;
 
  pr_debug(2, "<-- pthread start \n");
  
  s_data = (sec_info_t *)data;

  for(i=0; i<SEC_MAXTASKS; i++){
    count[i] = 1;
  } 

  s_data->th_pid = getpid();

  up_udelay(1000000); 
  
  while(exit_flag<SEC_MAXTASKS){ 
    for(i=0; i<SEC_MAXTASKS; i++)
      {
        s_etask = &(s_data->s_etask[i]);

        pr_debug(3, "wait for receiving a mq message \n");
        ret = mpmq_receive(&s_etask->mq, &msgdata);
        if (ret < 0)
          {
            err("mpmq_recieve() failure. %d\n", ret);
            goto err;  
          }
        else
          {
            switch (ret){
            case MSG_ID_SAYHELLO:
              message("received MSG_ID_SAYHELLO:%d \n",ret);
              dtype_d = (sec_mq_dtype_t *)msgdata;

              pr_debug(3,"recvd:%p from worker:%p \n",dtype_d->paddr, dtype_d->wpaddr);
              pr_debug(3,"Worker said:%s \n",((char *)(type_d->paddr) +0x20));
              count[i] = TEST_LOOP_COUNT; /* for exit */
              break;
            case MSG_ID_TESTMSG:
              message("received MSG_ID_TESTMSG:%d \n",ret);
              /* in case of OK */
              count[i]++;
              break; 
            default:
              err("error not support MSG_ID:%d \n",ret); 
            }
            pr_debug(3, "mpmq receive addr:%p \n", msgdata);
            vaddr  = mpshm_phys2virt(&s_etask->shm, (uintptr_t)msgdata);
            if(vaddr == NULL){
              err("error mpshm_phys2virt \n");
              return;
            }
            pr_debug(2, "vaddr:%p \n", vaddr);

            mpmutex_lock(s_etask->csem);
            message("task%d Worker said: %s\n", i, (char*)vaddr );

            mpmutex_unlock(s_etask->csem);

          }

        if(count[i] > TEST1_LOOP_COUNT)
          {
            /* msg exchange finished at this task */ 
            /* set flag */
            exit_flag++;
          }
        else
          {
            message("send a next message \n");
            /* send message next */

            ret = sec_send_nextmsg(s_data, i, count[i]);
            if(ret)
              {
                /* send error signal */
                ret = set_signal_err(s_data, i, s_data->my_pid); 
                if(ret) goto err; 
                goto skip;
              }
#if 0 /* test */
            else
              {  
                ret = set_signal_err(s_data, i, s_data->my_pid); 
                if(ret) goto err;
                goto skip;
              }
#endif
          }
      }  
  }
  ret = set_signal_as_rslt(s_data, s_data->my_pid);
  if(ret) goto err; 

skip: 
  pr_debug(2,"--> pthread end\n");
  return;
err:
  pr_debug(2, "--> pthread error end \n");
  return;
}
#endif
#ifdef _USE_THREAD
/****************************************************************************
 * Name: set_thread 
 ****************************************************************************/
static int set_thread(sec_info_t *s_data)
{
  int ret;
  pthread_attr_t thattr;
  struct sched_param       param;

  pthread_attr_init(&thattr);
  thattr.stacksize      = SEC_TH_STACKSIZE;
  param.sched_priority = SEC_TH_PRIORITY;
  pthread_attr_setschedparam(&thattr, &param);

//  ret = pthread_create(&monitor_th_tid, &thattr, (pthread_startroutine_t)sec_test_monitor5,
//                       (pthread_addr_t)(s_data));
  ret = pthread_create(&monitor_th_tid, &thattr, (pthread_startroutine_t)sec_test_monitor_sec,
                       (pthread_addr_t)(s_data));
  if (ret != 0)
    {
      ret = -ret; /* pthread_create does not modify errno. */
      goto err;
    }
   return 0;
err:
   return ret;

}
#endif

static uint32_t getlocalcpuid(void){
  return getreg32(CPU_ID);
}
static inline uint32_t mem_getactable(void)
{
  uint32_t cpuid = getlocalcpuid();

  return CXD56_ADR_CONV_BASE + (cpuid * 0x20) + 4;
}
uintptr_t mem_virt2phys(void *vaddr)
{
  uintptr_t va, pa;
  uint32_t reg;
  int8_t tag;

  va = (uintptr_t)vaddr >> 16;
  if (va & 0xfff0)
    {
      return 0;
    }
  tag = va & 0xf;

  reg = mem_getactable();
  pa = getreg32(reg + (4 * (tag / 2)));

  if (!(tag & 1))
    {
      pa <<= 16;
    }
  pa = (pa & 0x01ff0000u) | ((pa & 0x06000000) << 1);

  return pa | ((uintptr_t)vaddr & 0xffff);
}

void chk_test(sec_info_t *s_data){

 int addr=0xA5;
 uint32_t paddr;
 
 message("%s : addr(%%p):%p addr(%%x):%x \n",__func__,&addr, addr);

 paddr = (uint32_t)mpshm_virt2phys(NULL, (void *)&addr);
 pr_debug(3,"%s : paddr(%%p):%p paddr(%%x):%x \n",__func__, &paddr, paddr);

// treg = 0;

 return;

}

/****************************************************************************
 * Name: chk_mem_access 
 ****************************************************************************/
static int chk_mem_access(sec_info_t *s_data){
  int toffset;
  int i;
  int sz;
  int cnt;

  sz = s_data->ac_size;

  if(sz >TILE_OFFSET_SZ)
    {
      cnt = sz;
      sz = TILE_OFFSET_SZ;
    }
  else
    {
      cnt = sz;
    } 
  toffset = 0x0;

  toffset = s_data->tile_start;
  while(cnt>0) 
    {  

      pr_debug(3,"## CHECK START at %x \n",toffset); 
      pr_debug(3,"\n");

      for(i=0; i<sz; i++)
        {
          pr_debug(3,"%02x ",*(volatile char *)(TILE_BASE_ADDR+ toffset + i));
 
          if(!((i+1)%16)) pr_debug(3,"\n");
          if(!((i+1)%0x1000))
            {
              pr_debug(3,"## ADDR: %07x \n",TILE_BASE_ADDR + toffset +i);
//            pr_debug(3,"%02x ",*(volatile char *)(TILE_BASE_ADDR+ toffset + i));
//            if(!((i+1)%16)) pr_debug(3,"\n");
            }
          else
            {
              if(i>=TILE_OFFSET_SZ-0x10)
                {
                  pr_debug(3,"## ADDR: %07x \n",TILE_BASE_ADDR + toffset +i);
                  pr_debug(3,"%02x ",*(volatile char *)(TILE_BASE_ADDR+ toffset + i));
                  if(!((i+1)%16)) pr_debug(3,"\n");
                 }
            }
        }
      cnt -= sz; 
      if(cnt>0)
        {
          pr_debug(3,"## CHECK END at %x \n", (TILE_BASE_ADDR+ toffset + i));
        }
      else
        { 
          pr_debug(3, "## CHECK COMP \n");
        }
    }
  return 0; 
}
/****************************************************************************
 * Name: sec_multicpu_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int sec_multicpu1_4_main(int argc, char *argv[])
#endif
{
  int ret;
  sec_info_t sec_data;
  int flag = 0;

  /* get a parameter */
  set_area_addr(argc, argv, &sec_data);
 
  /* initialize */
  ret = init_param_secure(&sec_data);   
  if (ret)
    {
      err("init_param_secure setup failed %d \n",ret);
      flag = NG; 
      goto test_end;
    }
 
  ret = init_param(&sec_data);
  if (ret)
    {
      err("init_param setup failed %d \n",ret);
      flag = NG; 
      goto test_end;
    } 
  
  /* set a environment for ROMFS  */
  ret = set_rootfs(&sec_data);
  if (ret)
    {
      err("setup failed %d \n",ret);
      flag = NG; 
      goto test_end;
    }

#ifdef _PROT_TEST 
#ifdef _INTER_SUBCORE_TEST
  /* set some resources for each Worker */
  ret = set_param_worker_secure3(&sec_data);
#else
  /* set some resources for each Worker */
  ret = set_param_worker_secure(&sec_data);
#endif
#endif
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#ifdef _USE_SIGNAL 
  /* initialize for signal */
  ret = init_for_signal2(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#endif

#ifdef _PROT_TEST 
  ret = send_firstmsg_secure(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#ifdef _3RD_SECGRP_ENA
  ret = send_firstmsg(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#endif
#else
  ret = send_firstmsg(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#endif

#ifdef _USE_SIGNAL 
  ret = unmask_signal(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#endif

#ifdef _USE_THREAD
  ret = set_thread(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#endif
  up_udelay(500000);

#ifdef _3RD_SECGRP_ENA
   pr_dbg_workerinfo_n(&((sec_data.s_task)[0]), 10, PR_MEM_SZ, 0);
#endif
#if 0   
   for(i=0; i<SEC_MAXTASKS; i++){
     pr_dbg_workerinfo(&((sec_data.s_etask)[i]), 10+i, PR_MEM_SZ, 0);
   }
#endif
#ifdef _USE_WAIT_FIRSTMSG
  ret = wait_firstmsg(&sec_data);  
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
#endif

#ifdef _USE_WAITSIG
  ret = wait_signal(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
  pr_debug(1, "exit wait_signal \n");
#endif  

  up_udelay(1000000);
 
  printf("1st \n"); 
  chk_mem_access(&sec_data);

  up_udelay(3000000);

  printf("2nd \n"); 
  chk_mem_access(&sec_data);
#ifdef _USE_THREAD
  ret = pthread_join(monitor_th_tid, NULL);
  pr_debug(1, "exit pthread_join \n");
#endif

  ret = finalize_task_secure3(&sec_data);  
  if (ret)
    {
      flag = NG;
    } 
#ifdef _3RD_SECGRP_ENA
  ret = finalize_task(&sec_data);  
  if (ret)
    {
      flag = NG;
    } 
#endif
  if(flag==0)
    {
      message("Test Succeeded \n");
      return 0;
    } 
  else
    {
      message("Test Failed \n"); 
      return NG;
    }

test_end:
  return NG;
}
