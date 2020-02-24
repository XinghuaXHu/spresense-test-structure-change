/*
 * asmp_extensions_common.cxx
 *
 *  Created on: 2019年3月13日
 *      Author: uas
 */

#include "asmp_extensions_common.hpp"
#include <sys/stat.h>
#include <sys/mount.h>
#include <nuttx/drivers/ramdisk.h>
#include <stdio.h>
#include <errno.h>


#ifdef CONFIG_FS_ROMFS
#  include "worker/romfs.h"

#  define SECTORSIZE   512
#  define NSECTORS(b)  (((b)+SECTORSIZE-1)/SECTORSIZE)
#  define MOUNTPT "/romfs"
#endif

#ifndef MOUNTPT
#  define MOUNTPT "/mnt/vfat/BIN"
#endif

#define message(format, ...)    printf(format, ##__VA_ARGS__)
#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)

char g_workerpath[128];

mptask_info_t g_mptasks[WORKER_CPU_NUM];


bool app_init()
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
          return false;
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
    snprintf(g_workerpath, 128, "%s", MOUNTPT);
#else
    snprintf(g_workerpath, 128, "%s/%s", MOUNTPT, "HELLO");
#endif

    return true;
}

