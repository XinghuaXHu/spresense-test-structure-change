/****************************************************************************
 * worker_extensions.c
 *
 *   Copyright (C) 2016 Sony Corporation. All rights reserved.
 *   Author:
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

#include "asmp.h"

/* MP object keys. Must be synchronized with supervisor. */

#define KEY_SHM   1
#define KEY_MQ    2
#define KEY_MUTEX 3

enum MessageID
{
    MSG_ID_SAYHELLO = 1,
    MSG_ID_WRITE_SHM_START,
    MSG_ID_WRITE_SHM_DONE,
    MSG_ID_READ_SHM_START,
    MSG_ID_READ_SHM_DONE,
    MSG_ID_LOCK_START,
    MSG_ID_LOCK_DONE,
    MSG_ID_UNLOCK_START,
    MSG_ID_UNLOCK_DONE,
    MSG_ID_SEND,
    MSG_ID_SEND_DONE,
    MSG_ID_RECEIVE,
    MSG_ID_RECEIVE_DONE,
    MSG_ID_DO_SEND,
    MSG_ID_WORKER_QUIT,
};


#define ASSERT(cond) if (!(cond)) wk_abort()

static const char hellocpu[] = "Hello,CPU";

static char *strcopy(char *dest, const char *src)
{
  char *d = dest;
  while (*src) *d++ = *src++;
  *d = '\0';

  return dest;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
  mpmutex_t mutex;
  mpshm_t shm;
  mpmq_t mq[8];
  uint32_t msgdata;
  char *buf;
  int ret;

  /* Initialize MP Mutex */

  ret = mpmutex_init(&mutex, KEY_MUTEX);
  ASSERT(ret == 0);

  /* Initialize MP message queue,
   * On the worker side, 3rd argument is ignored.
   */

  ret = mpmq_init(&mq[3], 0, 3);
  ASSERT(ret == 0);
  ret = mpmq_init(&mq[4], 0, 4);
  ASSERT(ret == 0);
  ret = mpmq_init(&mq[5], 0, 5);
  ASSERT(ret == 0);
  ret = mpmq_init(&mq[6], 0, 6);
  ASSERT(ret == 0);
  ret = mpmq_init(&mq[7], 0, 7);
  ASSERT(ret == 0);

  /* Initialize MP shared memory */

  ret = mpshm_init(&shm, KEY_SHM, 1024);
  ASSERT(ret == 0);

  /* Map shared memory to virtual space */

  buf = (char *)mpshm_attach(&shm, 0);
  ASSERT(buf);

  /* Receive message from supervisor */
  mpmq_t* pMQ = 0;
  int myIndex;
  for( myIndex=3;myIndex<8;myIndex++)
  {
      if(asmp_getglobalcpuid()==myIndex)
      {
          pMQ = &mq[myIndex];
          break;
      }
  }

  ret = mpmq_receive(pMQ, &msgdata);

  while (ret > 0 && ret != MSG_ID_WORKER_QUIT)
  {
      switch (ret)
      {
      case MSG_ID_WRITE_SHM_START:
          /* Copy hello message to shared memory */
          strcopy(buf, hellocpu);
          buf[sizeof(hellocpu) - 1] = asmp_getglobalcpuid() + 0x30;
          buf[sizeof(hellocpu)] = '\0';

          /* Send done message to supervisor */
          msgdata = (uint32_t)buf;
          ret = mpmq_send(pMQ, MSG_ID_WRITE_SHM_DONE, msgdata);
          ASSERT(ret == 0);
          break;
      case MSG_ID_READ_SHM_START:
          {
              if((buf[0] == 'H') && (buf[1] == 'e') &&(buf[2] == 'l') &&(buf[3] == 'l') && (buf[4] == 'o') &&
                 (buf[5] == ',') &&(buf[6] == 'C') &&(buf[7] == 'P') &&(buf[8] == 'U') &&(buf[9] == (msgdata + 0x30)))
              {
                  ret = mpmq_send(pMQ, MSG_ID_READ_SHM_DONE, msgdata);
                  ASSERT(ret == 0);
              }
          }
          break;
      case MSG_ID_LOCK_START:
          mpmutex_lock(&mutex);
          msgdata = 0;
          ret = mpmq_send(pMQ, MSG_ID_LOCK_DONE, 0);
          break;
      case MSG_ID_UNLOCK_START:
          mpmutex_unlock(&mutex);
          msgdata = 0;
          ret = mpmq_send(pMQ, MSG_ID_UNLOCK_DONE, 0);
          break;
      case MSG_ID_SEND:
      {
          if(msgdata != myIndex)
          {
              uint32_t data = 0xffffffff;
              mpmq_send(&mq[msgdata], MSG_ID_DO_SEND, data);
              mpmq_send(pMQ, MSG_ID_SEND_DONE, data);
          }
      }
      break;
      case MSG_ID_RECEIVE:
      {
         if(msgdata != myIndex)
         {
             if(MSG_ID_DO_SEND == mpmq_receive(&mq[msgdata],&msgdata))
             {
                 mpmq_send(pMQ, MSG_ID_RECEIVE_DONE, msgdata);
             }
         }
      }
      break;
      default:
          break;
      }

      ret = mpmq_receive(&mq, &msgdata);
  }

  /* Free virtual space */
  mpshm_detach(&shm);

  return 0;
}

