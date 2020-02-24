/****************************************************************************
 * test/sqa/singlefunction/multicpu_sec/memory_security.c
 *
 *   Copyright 2018,2019 Sony Semiconductor Solutions Corporation
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
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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

#include <asmp/asmp.h>
#include <asmp/mptask.h>
#include <asmp/mpshm.h>
#include <asmp/mpmq.h>
#include <asmp/mpmutex.h>


//#define _T_TEST_

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
#  define MOUNTPT "/mnt/vfat/BIN"
#endif

#define TEST_TILE_SIZE (128*1024)
#define TEST_TILE_SIZE2 (64*1024)
#define TEST_TILE_CHECK_OFFSET  0x500
#define TEST_TILE_CHECK2_OFFSET 0x1000

/* Check configuration.  This is not all of the configuration settings that
 * are required -- only the more obvious.
 */

#if CONFIG_NFILE_DESCRIPTORS < 1
#  error "You must provide file descriptors via CONFIG_NFILE_DESCRIPTORS in your configuration file"
#endif

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)

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
static char fullpath[128];

/****************************************************************************
 * Symbols from Auto-Generated Code
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int start_mptask(const char *filename, mptask_t *task, bool secure)
{
  int ret;

  /* Initialize MP task */

  if (secure)
    {
      ret = mptask_init_secure(task, filename);
      if (ret != 0)
        {
          err("mptask_init_secure() failure. %d\n", ret);
          return ret;
        }
    }
  else
    {
      ret = mptask_init(task, filename);
      if (ret != 0)
        {
          err("mptask_init() failure. %d\n", ret);
          return ret;
        }
    }

  ret = mptask_assign(task);
  if (ret != 0)
    {
      err("mptask_asign() failure. %d\n", ret);
      return ret;
    }

  message("assigned at CPU%d\n", mptask_getcpuid(task));

  /* Run worker */

  ret = mptask_exec(task);
  if (ret < 0)
    {
      err("mptask_exec() failure. %d\n", ret);
      return ret;
    }

#ifdef _T_TEST_
  printf("task->loadaddr:0x%08x \n",task->loadaddr);
  printf("task->loadsize:0x%x \n",task->loadsize);
#endif      
  return 0;
}

static int end_mptask(mptask_t *task)
{
  int ret, wret;

  /* Destroy worker */

  wret = -1;
  ret = mptask_destroy(task, false, &wret);
  if (ret < 0)
    {
      err("mptask_destroy() failure. %d\n", ret);
      return ret;
    }

  message("Worker exit status = %d\n", wret);

  return 0;
}

static int update_step(char *step)
{
  FILE *fp;
  int  fd;

  fp = fopen("/mnt/spif/mem_security", "w");
  fd = fileno(fp);
  fwrite(step, 1, 1, fp);
  fflush(fp);
  fsync(fd);
  fclose(fp);

  return 0;
}

static int check_mem_security(mptask_t *task, mptask_t *task2)
{
  unsigned long *addr = (unsigned long *)CONFIG_RAM_START;
  unsigned long tsize;
  char step;
  FILE *fp;

  /* Calculate allocated tile size */

  tsize = task->loadsize + (TEST_TILE_SIZE - 1);
#ifdef _T_TEST_
  printf("tsize:0x%08x \n", tsize);
#endif
  tsize &= ~(TEST_TILE_SIZE - 1);

#ifdef _T_TEST_
  printf("mask:0xx \n", ~(TEST_TILE_SIZE -1));
  printf("tsize:0x%08x \n", tsize);
#endif
  /* mem_security file has the next point that this application should execute */

  fp = fopen("/mnt/spif/mem_security", "r");
  if (fp != NULL)
    {
      /* If exist, read address */

      fread(&step, 1, 1, fp);

      /* Re-open with write-only mode */

      fclose(fp);

      /* Jump to step of mem_security file */

      switch (step)
        {
          case '0':
            goto step0;

          case '1':
            goto step1;

          case '2':
            goto step2;

          case '3':
            goto step3;

          case '4':
            goto step4;

          case '5':
            goto step5;
#if 0
          case '6':
            goto step6;
#endif
          default:

            /* no jump */

            break;
        }
    }

step0:
  /* Update memory_security(next, start from step1) */

  update_step("1");

  /* small address */

  printf("Start: small address\n");

  for (addr = (unsigned long *)CONFIG_RAM_START;
       addr < (unsigned long *)task->loadaddr;
       addr = (unsigned long *)(addr + 0x10000))
    {
      printf("Access dekita. 0x%08x : value=0x%08x\n", addr, *addr);
    }

  printf("End: small address \n");

step1:
  /* Update memory_security(next, start from step2) */

  update_step("2");

  /* Just before load address */

  printf("Start: Just before load address\n");
 
  addr = (unsigned long *)(task->loadaddr - 4);
  printf("Access dekita. 0x%08x : value=0x%08x\n", addr, *addr);

  printf("End: Just before load address\n");

step2:
  /* Update memory_security(next, start from step3) */

  update_step("3");

  /* top of load address */

  printf("Start: top of load address\n");

  addr = (unsigned long *)task->loadaddr;

  printf("Access dekita. 0x%08x : value=0x%08x\n", addr, *addr);

  /* In secure case, koko made konai is kitai */

  printf("End: top of load address\n");

step3:
  /* Update memory_security(next, start from step4) */

  update_step("4");

  /* middle of load address */

  printf("Start: middle of load address\n");

  addr = (unsigned long *)(task->loadaddr + (tsize/2));

  printf("Access dekita. 0x%08x : value=0x%08x\n", addr, *addr);

  /* In secure case, koko made konai is kitai */

  printf("End: middle of load address\n");

step4:
  /* Update memory_security(next, start from step5) */

  update_step("5");

  /* end of load address */

  printf("Start: end of load address\n");

  addr = (unsigned long *)(task->loadaddr + tsize - 1);


  printf("Access dekita. 0x%08x : value=0x%08x\n", addr, *addr);

  /* In secure case, koko made konai is kitai */

  printf("End: end of load address\n");

step5:
  /* Update memory_security(next, start from step6) */

  update_step("6");

  /* Just after load address */
  printf("Start: non-secure area address\n");

  for (addr = (unsigned long *)(task->loadaddr + tsize );
       addr < (unsigned long *)(task2->loadaddr + tsize);
       addr = (unsigned long *)(addr + 0x1000))
    {
      printf("Access dekita. 0x%08x : value=0x%08x\n", addr, *addr);
    }
  printf("End: non-secure area address\n");

  /* remove test file */

  remove("/mnt/spif/mem_security");

  return 0;
}

static void check_mem_content(mptask_t *task)
{
  int    cnt;
  size_t tsize;
  unsigned long *addr;
  unsigned long *addrend;
  signed long tmpval=0;

  /* Calculate allocated tile size */

  tsize = task->loadsize + (TEST_TILE_SIZE - 1);
  tsize &= ~(TEST_TILE_SIZE - 1);

  addr = (unsigned long *)task->loadaddr;
  addrend = (unsigned long *)(task->loadaddr + TEST_TILE_SIZE);

#ifdef _T_TEST_
  printf("addr:%lu (HEX: %08x) \n", addr, addr);
  printf("addrend:%lu (HEX: %08x) \n", addrend, addrend);
  printf("diff:%ld (HEX: %08x) \n", (signed long)addrend - (signed long)addr, (signed long)addrend - (signed long)addr);
#endif

  printf("Start: check memory contents\n");

  printf("0x%08x: ", addr);

  for (cnt = 0; addr < addrend; cnt++)
    {
      if(addr < (unsigned long *)(task->loadaddr + TEST_TILE_CHECK_OFFSET)){
        printf("%08x  ", *addr);
        if ((cnt+1)%8 == 0)
          {
            printf("\n");
            addr++;
            printf("0x%08x: ", addr);
          }
        else
          {
            addr++; 
          }
      }
      else if(addr >= (unsigned long *)TEST_TILE_CHECK_OFFSET && 
        addr < (unsigned long *)(task->loadaddr + TEST_TILE_CHECK2_OFFSET)){
        if(*addr != 0x00){
          printf("## Error! not expected data 0x%x \n", *addr);
          printf("0x%08x: ", addr);
          printf("%08x  \n", *addr);
          addr++;
          goto fail;
        }
        else
        {
          tmpval = (signed long)addr - (signed long)(task->loadaddr);
          printf("\r status : %.2f (%%)", (((float)tmpval)/(float)TEST_TILE_SIZE)*100);
          addr++;
        }
      }
      else
      {
        if(*addr == 0xFEFEFEFE){
          printf("## Error! some secure data is leaked : 0x%x\n", *addr);
          printf("0x%08x: ", addr);
          printf("%08x  \n", *addr);
          addr++;
          goto fail;
        }
        else
        {
          tmpval = (signed long)addr - (signed long)(task->loadaddr);
          //printf("tmpval:%ld \n",tmpval);
          printf("\r status : %.2f (%%)", (((float)tmpval)/(float)TEST_TILE_SIZE)*100);
          addr++;
        }
      }
    }
  printf("\n");
  printf("End: check memory contents\n");

  printf("Test finished successfully!! \n");
  return;
fail:
  printf("Test failed!! \n");
  return;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int multicpu_memory_security(void)
{
  mptask_t task;
  mptask_t task2;

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
        }
    }
#endif

#ifdef CONFIG_FS_ROMFS
    snprintf(fullpath, 128, "%s/%s", MOUNTPT, "hello_noproc");
#else
    snprintf(fullpath, 128, "%s/%s", MOUNTPT, "HELLO_NOPROC");
#endif

  /* load secure image */

  start_mptask("SECFREE", &task, true);
  start_mptask(fullpath, &task2, false);

  /* Check memory security */

  check_mem_security(&task, &task2);

  /* unload secure image */

  end_mptask(&task);

  /* load non-secure image */

  start_mptask(fullpath, &task, false);

  /* Check memory content */

  check_mem_content(&task);

  /* unload non-secure image */

  end_mptask(&task);
  end_mptask(&task2);

  return OK;
}
