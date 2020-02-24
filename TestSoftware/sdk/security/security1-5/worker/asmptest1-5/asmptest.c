/****************************************************************************
 * worker/secutest1-1/sectest1-1.c 
 *
 *   Copyright (C) 2016 Sony Corporation. All rights reserved.
 *   Author: Nobuto Kobayashi <Nobuto.Kobayashi@sony.com>
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
#include <errno.h>
#include <asmp/types.h>
#include <asmp/mpshm.h>
#include <asmp/mpmutex.h>
#include <asmp/mpmq.h>
#include <asmp.h>
//#include "asmp.h"
#include "dummy_data.inc"

#define WDEBUG_ON
#define WDEBUG_ON2

/* MP object keys. Must be synchronized with supervisor. */

#define KEY_SHM   1
#define KEY_MQ    2
#define KEY_MUTEX 3

#define MSG_ID_SAYHELLO 1
#define MSG_ID_TESTMSG  2

#define TEST_LOOP_COUNT 5 
#define MSG_IND_CNT  21
#define PHYSADDR_MASK 0x0DF0000
#define _TEST_NONSECURE 

#define TARGET_ADDR 0x0d080000
//#define TARGET_ADDR 0x0d0e0000
//#define TARGET_ADDR 0x0d0f0000
//#define TARGET_ADDR 0x0d100000
//#define TARGET_ADDR 0x0d120000
//#define TARGET_ADDR 0x0d110000
//#define TARGET_ADDR 0x0d090000

#define ASSERT(cond) if (!(cond)) wk_abort()

static const char hellocpu[] = "Hello, CPU";

static char *strcopy(char *dest, const char *src)
{
  char *d = dest;
  while (*src) *d++ = *src++;
  *d = '\0';

  return dest;
}
typedef struct sec_mq_dtype{
  uintptr_t paddr;
  uintptr_t wpaddr;
  size_t    size;
  int       data;
}sec_mq_dtype_t;


unsigned char wptr =0;
/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
  mpmutex_t mutex;
  mpshm_t shm;
  mpmq_t mq;
  uint32_t msgdata;
#ifdef WDEBUG_ON
  uint32_t tdata;
#endif
  char *buf;
  int ret;
  int i;
  volatile int val=0;
  sec_mq_dtype_t *super_shm;
  sec_mq_dtype_t test_shm;
  uintptr_t taddr;

#ifdef _TEST_NONSECURE 
  /* Initialize MP Mutex */
  ret = mpmutex_init(&mutex, KEY_MUTEX);
  ASSERT(ret == 0);
#endif

  /* Initialize MP message queue,
   * On the worker side, 3rd argument is ignored.
   */

  ret = mpmq_init(&mq, KEY_MQ, 0);
  ASSERT(ret == 0);

#ifdef _TEST_NONSECURE 
  /* Initialize MP shared memory */
  ret = mpshm_init(&shm, KEY_SHM, 1024);
  ASSERT(ret == 0);

  /* Map shared memory to virtual space */

  buf = (char *)mpshm_attach(&shm, 0);
  ASSERT(buf);

#ifdef WDEBUG_ON
  if((buf[0] != 0x00) || (buf[1] != 0x00)) goto error;
#endif
#endif

#ifdef WDEBUG_ON
  mpmutex_lock(&mutex);
  buf[wptr++] = 0xF0;
  buf[wptr++] = 0x11;
  mpmutex_unlock(&mutex);
#endif

  /* Receive message from supervisor */

  ret = mpmq_receive(&mq, &msgdata);
  ASSERT(ret == MSG_ID_SAYHELLO);

  super_shm = (sec_mq_dtype_t *)&msgdata;
  /* physical adress for a shared memory prepared bt supervisor*/  
//  test_shm.paddr = super_shm->paddr; 
//  test_shm.paddr = 0xd060000 + 0x20000*(asmp_getglobalcpuid()-3); 
#if 0

  test_shm.paddr = 0xd060000; 
  test_shm.wpaddr = super_shm->paddr; 

  ((volatile char *)super_shm->paddr)[0x30] = 'H';
  ((volatile char *)super_shm->paddr)[0x31] = 'e';
  ((volatile char *)super_shm->paddr)[0x32] = 'l';
  ((volatile char *)super_shm->paddr)[0x33] = 'l';
  ((volatile char *)super_shm->paddr)[0x34] = 'o';
  ((volatile char *)super_shm->paddr)[0x35] = ',';
  ((volatile char *)super_shm->paddr)[0x36] = asmp_getglobalcpuid() + 0x30;
  ((volatile char *)super_shm->paddr)[0x37] = '\0';
  
  ((volatile char *)test_shm.paddr)[0x30] = 0xA5;
  ((volatile char *)test_shm.paddr)[0x31] = 0x39;
#endif
  buf[0x30] = (char)(shm.paddr);
  buf[0x31] = (char)(shm.paddr >>8);
  buf[0x32] = (char)(shm.paddr >>16);
  buf[0x33] = (char)(shm.paddr >>24);
  taddr = (uintptr_t)mpshm_virt2phys(NULL, &buf); 
  buf[0x40] = (char)taddr;
  buf[0x41] = (char)(taddr>>8);
  buf[0x42] = (char)(taddr>>16);
  buf[0x43] = (char)(taddr>>24);

//  msgdata = (uint32_t)mpshm_virt2phys(&shm, (void *)(buf)); 


  mpmutex_lock(&mutex);
  buf[wptr++] = 0xF0;
  buf[wptr++] = 0x12;
  mpmutex_unlock(&mutex);

  val = TARGET_ADDR;
  
  mpmutex_lock(&mutex);
  buf[0x50] = (char)val;
  buf[0x51] = (char)(val>>8);
  buf[0x52] = (char)(val>>16);
  buf[0x53] = (char)(val>>24);
  mpmutex_unlock(&mutex);

  for(i=0; i<0x10000; i=i+4){
//  val = *((volatile char *)(0x0d0e0000));
    val = *((volatile int *)(TARGET_ADDR + i));
  
    mpmutex_lock(&mutex);
    buf[0x54] = (char)val;
    buf[0x55] = (char)(val>>8);
    buf[0x56] = (char)(val>>16);
    buf[0x57] = (char)(val>>24);
  
    buf[0x58] = (char)i;
    buf[0x59] = (char)(i>>8);
    buf[0x5A] = (char)(i>>16);
    buf[0x5B] = (char)(i>>24);
    mpmutex_unlock(&mutex);
  }
  mpmutex_lock(&mutex);
    buf[0x5C] = 0x45;
    buf[0x5D] = 0x23;
    buf[0x5E] = 0x01;
    buf[0x5F] = 0x38;
//  buf[wptr++] = 0xF0;
//  buf[wptr++] = 0x13;
  mpmutex_unlock(&mutex);

#ifdef _TEST_NONSECURE
  ret = mpmq_send(&mq, MSG_ID_SAYHELLO, msgdata);
  ASSERT(ret == 0);

  for(i=0; i<=TEST_LOOP_COUNT; i++)
    {
      ret = mpmq_receive(&mq, &msgdata);
      ASSERT(ret == MSG_ID_TESTMSG);
  
      val = ((char *)msgdata)[MSG_IND_CNT] - 0x30; 
      ((char *)msgdata)[MSG_IND_CNT] = val++ + 0x30;
  
      ret = mpmq_send(&mq, MSG_ID_TESTMSG, msgdata);
      ASSERT(ret == 0);
    }

  /* Free the virtual space */
  mpshm_detach(&shm);
#else
  ret = mpmq_send(&mq, MSG_ID_SAYHELLO, test_shm.paddr);
  ASSERT(ret == 0);
   
#endif
  return 0;
#ifdef WDEBUG_ON
error:
  return 1;
#endif
}
