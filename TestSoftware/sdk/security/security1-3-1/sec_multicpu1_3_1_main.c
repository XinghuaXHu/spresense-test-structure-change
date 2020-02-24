/****************************************************************************
 * test/sqa/singlefunction/security1-3/sec_multicpu1_3_1_main.c
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
//#define _USE_MULTI_MPMUTEX 
//#define _USE_WAITSIG

#define SEC_TH_STACKSIZE 2048 
#define SEC_TH_PRIORITY  SCHED_PRIORITY_DEFAULT 
//#define _USE_THREAD
//#define _TEST_SIG

#define _ASMP_SEC

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

#define DEBUG_LEVEL  3 

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
static int setup_task_secure(const char *, sec_info_t *);
static int finalize_task_secure2(sec_info_t *);
static int start_task_secure(sec_info_t *);
static int set_rootfs(sec_info_t *);
static int set_param_worker_secure(sec_info_t *);
//static int send_firstmsg_secure(sec_info_t *);

#ifdef _USE_THREAD
static FAR void sec_test_monitor_sec(FAR void *);
static int set_signal_asrslt(sec_info_t *, pid_t); 
#endif

/****************************************************************************
 * Debug Functions 
 ****************************************************************************/
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

  pr_debug(3,"Memory %d\n", i);

  buf = (char *)(s_etask->buf); 
  pr_debug(2,"init buf:%p \n",buf); 

  if(buf == NULL)
    {
       message("\n");
       return;
    }
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

  buf = (char *)(s_etask->buf);
  pr_debug(2, "init buf:%p \n",buf); 

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
  return;
}

void pr_dbg_workerinfo(sec_etask_t *s_etask, int i, int sz, int offset)
{
  char *buf;
  int m;

  pr_debug(3,"Memory %d\n", i);

  buf = (char *)(s_etask->buf); 
  pr_debug(2,"init buf:%p \n",btauf); 

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
  
  for(i=0; i<SEC_MAXTASKS; i++)
    {
      s_data->s_etask[i].cpuno = i; 
      s_data->s_etask[i].buf = NULL; 
    } 
//  sec_init_tdata(s_data);

  return 0;
}

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
  
  for(i=0; i<SEC_MAXTASKS; i++){ 
    s_etask = &s_data->s_etask[i];

    /* Initialize MP message queue with asigned CPU ID, and bind it to MP task */

    if(CPU_ISSET(i+CPUNO_OFFSET, &combd->cpuids))
      {
        s_etask->cpuno = i+CPUNO_OFFSET;
        pr_debug(3, "cpuno:%d \n", s_etask->cpuno); 
        s_etask->shm = s_data->shm;
        pr_debug(2,"s_etask->shm:%p s_data->shm:%p \n",s_etask->shm, s_data->shm); 
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
        if(i==0)
          { 
            /* Note: a error occured in mpshm_attach at 2nd times using s_etask->shm  */
            s_etask->buf = (char *)mpshm_attach(&s_data->shm, 0);
            if(s_etask->buf==NULL)
              {
                err("mpshm_attach error!! \n"); 
                return ret;
              } 
            pr_dbg_initmem_sec(s_etask, SEC_SZ_SHM);
          }
        else
          {
            s_etask->buf = (s_data->s_etask[0]).buf;
          }
      }
   }

  return 0;
}

/****************************************************************************
 * finalize task 
 ****************************************************************************/
static int finalize_task_secure2(sec_info_t *s_data)
{
  int ret;
  int wret;
  sec_combinfo_t *s_comb;

  s_comb = &s_data->s_comb;
  
  pr_debug(2,"==> %s \n",__func__); 

  /* Exit worker */

  pr_debug(2,"task:%p \n",&s_comb->task);
  pr_debug(2,"comb:%p \n",s_comb);

  /* Finalize MP task */
 
  wret = OK;

  /* true means forcibly shutdown */
  ret = mptask_destroy(&s_comb->task, true, &wret);
  if (ret < 0)
    {
      err("mptask_destroy() failure. %d\n", ret);
      return ret;
    }

  message("Worker exit status = %d\n", wret);

  mpshm_detach(&s_data->shm);
  mpshm_destroy(&s_data->shm);
  mpmutex_destroy(&s_data->sem);

  pr_debug(2,"<== %s \n",__func__); 
  return 0;
}

/****************************************************************************
 * start_task_secure 
 ****************************************************************************/
static int start_task_secure(sec_info_t *s_data)
{
  int ret;
  sec_combinfo_t *s_comb;

  s_comb = &s_data->s_comb;

  /* Run worker */

  pr_debug(2,"task:%p \n",&s_comb->task);
  pr_debug(2,"comb:%p \n",s_comb);
  pr_debug(2,"s_etask.task:%p \n",&s_data->s_comb.task);
  pr_debug(2,"s_etask:%p \n",&s_data->s_comb.task);

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
    snprintf(s_data->fullpath, 128, "%s", "ledtest");
#endif

  pr_debug(3, "%s : process OK \n",__func__); 
  return 0;

}

/****************************************************************************
 * Name: sec_param_worker
 ****************************************************************************/
static int set_param_worker_secure(sec_info_t *sec_data)
{
  int i=0;
  int ret;

  /* Initialize a common MP mutex for all tasks */

  ret = mpmutex_init(&sec_data->sem, KEY_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }

  /* mpshm_init() */
  ret = mpshm_init(&sec_data->shm, KEY_SHM, SEC_SZ_SHM);
  if (ret < 0)
    {
      err("mpshm_init() failure. %d\n", ret);
      return ret;
    }
  /* setup a task for secure  */
  ret = setup_task_secure(sec_data->fullpath, sec_data);
  if(ret)
    {
       goto test_fail;
    }  
  ret = start_task_secure(sec_data);
  if(ret)
    {
      goto test_fail;
    }
  else
    {
      pr_debug(3, "task%d started \n",i); 
    }
  return 0;
test_fail:
  return NG;
}

/****************************************************************************
 * Name: sec_multicpu_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int sec_multicpu1_3_1_main(int argc, char *argv[])
#endif
{
  int ret;
  sec_info_t sec_data;
  int flag = 0;
 
  /* initialize */
  ret = init_param_secure(&sec_data);   
  if (ret)
    {
      err("setup failed %d \n",ret);
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
  /* set some resources for each Worker */
  ret = set_param_worker_secure(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }
  up_udelay(1000000); 

  pr_dbg_workerinfo(&((sec_data.s_etask)[0]), 10, 0x100, 0);
 
  up_udelay(1000000); 
  ret = finalize_task_secure2(&sec_data);  
  if (ret)
    {
      flag = NG;
    } 
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
