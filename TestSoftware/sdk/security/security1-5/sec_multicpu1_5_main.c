/****************************************************************************
 * test/sqa/singlefunction/security1-5/sec_multicpu1_5_main.c
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
#include "sec_struct.h"

#define SEC_TH_STACKSIZE 2048 
#define SEC_TH_PRIORITY  SCHED_PRIORITY_DEFAULT 

#define _PROT_TEST

#define putreg32(val, addr) *(volatile uint32_t *)(addr) = (val)
#define getreg32(addr)      *(volatile uint32_t *)(addr)
#define wraddr8(val, addr) *(volatile char *)(addr) = (val)
#define wraddr32(val, addr) *(volatile uint32_t *)(addr) = (val)

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

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#if CONFIG_NFILE_DESCRIPTORS < 1
#  error "You must provide file descriptors via CONFIG_NFILE_DESCRIPTORS in your configuration file"
#endif

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)
#define pr_debug(dbg,format, ...)   {if(dbg>=DEBUG_LEVEL) printf(format, ##__VA_ARGS__);}

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
static int init_param(sec_info_t *);
static int set_rootfs(sec_info_t *);
static int setup_task_n(const char *, sec_info_t *, int);
static int setup_task_n2(const char *, sec_info_t *, int);
static int start_task(sec_task_t *);
static int finalize_task(sec_info_t *);
static int chk_mem_access(sec_info_t *);
#ifndef _NO_USE_SET_ADDR_
static int set_area_addr(int , char *[], sec_info_t *);
#endif

char tbuffer[100];

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

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static int init_param(sec_info_t *s_data)
{
  int i;

  s_data->fullpath3[0] = '\0';
  s_data->fullpath4[0] = '\0';

  for(i=0; i<MAXTASKS; i++)
    {
      s_data->s_task[i].buf = NULL; 
    } 

  return 0;
}
#ifndef _NO_USE_SET_ADDR_
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
      message("argc:%d \n",argc); 
      s_data->tile_start = TILE_OFFSET_BASE; /* offset address for start */ 
    }
  return 0; 
}
#endif

/****************************************************************************
 * setup_task_n 
 ****************************************************************************/
static int setup_task_n(const char *filename, sec_info_t *sec_data, int no)
{
  int ret;
  void *virtaddr=NULL;
  sec_task_t *s_task;
  sec_task_t *s_task2;

  pr_debug(2,"==> %s no:%d \n",__func__);
 
  /* Initialize MP task */

  s_task = &(sec_data->s_task[no]); 
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

  /* bind it to MP task */
  ret = mptask_bindobj(&s_task->task, s_task->csem);
  
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

  if(no ==0){
 
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
  }
  else{
    s_task2 = &(sec_data->s_task[0]); 
    s_task->buf = s_task2->buf;
    if(s_task->buf==NULL){
      err("mpshm_attach error!! \n"); 
      return ret;
    } 
    ret = mptask_bindobj(&s_task->task, &s_task2->shm);
    if (ret < 0)
      {
        err("mptask_bindobj(shm) failure. %d l:%d \n", ret, __LINE__);
        return ret;
      }
     
  }    

  return 0;
}


/****************************************************************************
 * setup_task_n2 
 ****************************************************************************/
static int setup_task_n2(const char *filename, sec_info_t *sec_data, int no)
{
  int ret;
  sec_task_t *s_task;
  sec_task_t *s_task2;

  pr_debug(2,"==> %s no:%d \n",__func__);
 
  /* Initialize MP task */

  s_task = &(sec_data->s_task[no]); 
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

  /* bind it to MP task */
  ret = mptask_bindobj(&s_task->task, s_task->csem);
  
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

  if(no ==0){

    s_task->buf = (char *)tbuffer;

  }
  else{
    s_task2 = &(sec_data->s_task[0]); 
    s_task->buf = s_task2->buf;
    if(s_task->buf==NULL){
      err("mpshm_attach error!! \n"); 
      return ret;
    } 
  }    

  return 0;
}

/****************************************************************************
 * finalize task 
 ****************************************************************************/
static int finalize_task(sec_info_t *s_data)
{
  int ret;
  int wret;
  sec_task_t *task;
  int i;

  pr_debug(3,"==> %s \n",__func__); 

  for(i=0; i<MAXTASKS; i++){
    /* Finalize MP task */
    task = &(s_data->s_task[i]);
 
    wret = OK;

    /* true means forcibly shutdown */

    printf("task:%p \n",task->task);

    ret = mptask_destroy(&(task->task), true, &wret);
    if (ret < 0)
      {
        err("mptask_destroy() failure. %d\n", ret);
        return ret;
      }

    message("Worker exit status = %d\n", wret);
  }
  task = &(s_data->s_task[0]);
  mpmq_destroy(&task->mq);
  mpshm_detach(&task->shm);  
  mpshm_destroy(&task->shm);
  
  mpmutex_destroy(&(s_data->sem));

  pr_debug(3,"<== %s \n",__func__); 
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
 * set_rootfs 
 ****************************************************************************/
static int set_rootfs(sec_info_t *s_data)
{

  pr_debug(3, "%s : ==> \n",__func__); 

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

    snprintf(s_data->fullpath3, 128, "%s%s", "/mnt/sd0/","asmptest");
    snprintf(s_data->fullpath4, 128, "%s%s", "/mnt/sd0/","asmptest2");

  pr_debug(3, "%s : process OK \n",__func__); 
  return 0;

}

#ifdef _PROT_TEST 
/****************************************************************************
 * Name: sec_param_worker
 ****************************************************************************/
static int set_param_worker(sec_info_t *sec_data)
{
  int i=0;
  int ret;

  pr_debug(2, "%s ==> ",__func__);
  /* Initialize a common MP mutex for all tasks */
  ret = mpmutex_init(&sec_data->sem, KEY_MUTEX);
  if (ret < 0)
    {
      err("mpmutex_init() failure. %d\n", ret);
      return ret;
    }

  /* final TEST target */

  for(i=0; i<4; i++){ 
    
    /* setup a task for non-secure */
    sec_data->s_task[i].csem = &(sec_data->sem);

    ret = setup_task_n(sec_data->fullpath3, sec_data, i);
//    ret = setup_task_n2(sec_data->fullpath3, sec_data, i);
    if(ret)
      {
        goto test_fail;
      }
  }

  for(i=0; i<4; i++){ 
    pr_dbg_initmem(&(sec_data->s_task[i]), SEC_SZ_SHM);
    ret = start_task(&(sec_data->s_task[i]));
    if(ret)
      {
        goto test_fail;
      }
    else
      {
        message("non secure task started \n");
      }
  } 
    /* setup a task for non-secure */
    sec_data->s_task[4].csem = &(sec_data->sem);
    ret = setup_task_n(sec_data->fullpath4, sec_data, 4);
//    ret = setup_task_n2(sec_data->fullpath4, sec_data, 4);
    if(ret)
      {
        goto test_fail;
      }
    pr_dbg_initmem(&(sec_data->s_task[4]), SEC_SZ_SHM);
    ret = start_task(&(sec_data->s_task[4]));
    if(ret)
      {
        goto test_fail;
      }
    else
      {
        message("non secure task started \n");
      }
  pr_debug(2, "%s <== OK",__func__);
  return 0;
test_fail:
  pr_debug(2, "%s <==  NG",__func__);
  return NG;
}
#endif

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
          pr_debug(3,"%02x ",*(volatile char *)(0xd000000+ toffset + i));
 
          if(!((i+1)%16)) pr_debug(3,"\n");
          if(!((i+1)%0x1000))
            {
              pr_debug(3,"## ADDR: %07x \n",0xd000000 + toffset +i);
            }
          else
            {
              if(i>=TILE_OFFSET_SZ-0x10)
                {
                  pr_debug(3,"## ADDR: %07x \n",0xd000000 + toffset +i);
                  pr_debug(3,"%02x ",*(volatile char *)(0xd000000+ toffset + i));
                  if(!((i+1)%16)) pr_debug(3,"\n");
                 }
            }
        }
      cnt -= sz; 
      if(cnt>0)
        {
          pr_debug(3,"## CHECK END at %x \n", (0xd00000+ toffset + i));
        }
      else
        { 
          pr_debug(3, "## CHECK COMP \n");
        }
    }
  return 0; 
}
/* test to check finalize */

/****************************************************************************
 * Name: sec_multicpu_main
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int sec_multicpu1_5_main(int argc, char *argv[])
#endif
{
  int ret;
  sec_info_t sec_data;
  int flag = 0;

#ifndef _NO_USE_SET_ADDR_
  /* get a parameter */
  set_area_addr(argc, argv, &sec_data);
#endif 
  /* initialize */
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

  /* set some resources for each Worker */
  ret = set_param_worker(&sec_data);
  if (ret)
    {
      flag = NG; 
      goto test_end;
    }

   pr_dbg_workerinfo_n(&((sec_data.s_task)[0]), 10, PR_MEM_SZ, 0);

  up_udelay(1000000);
 
  printf("1st \n"); 
  chk_mem_access(&sec_data);

  up_udelay(3000000);

  printf("2nd \n"); 
  chk_mem_access(&sec_data);

  ret = finalize_task(&sec_data);  
  if (ret)
    {
      flag = NG;
    } 
   return 0;
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
