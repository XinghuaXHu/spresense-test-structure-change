/****************************************************************************
 * sqa/combination/player_gnss_led_camera_asmp/led.c
 *
 *   Copyright (C) 2017 Sony Corporation
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
#if 0
#include <sys/types.h>
#include <sys/ioctl.h>
#endif
#include <pthread.h>

#include <stdio.h>
// #include <fcntl.h>

#include <arch/board/board.h>
#include <arch/chip/pin.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* LED ID */

#define LED0    (uint8_t)(0)
#define LED1    (uint8_t)(1)
#define LED2    (uint8_t)(2)
#define LED3    (uint8_t)(3)

/* LED GPIO value */

#define LOW     0x0
#define HIGH    0x1

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint8_t led_pincvrt[] =
{
  PIN_I2S1_BCK,        /* LED0 */
  PIN_I2S1_LRCK,       /* LED1 */
  PIN_I2S1_DATA_IN,    /* LED2 */
  PIN_I2S1_DATA_OUT    /* LED3 */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void led_main(uint8_t led)
{
  uint8_t pin = led_pincvrt[led];

  board_gpio_write(pin, HIGH);
  usleep(50*1000);  /* 50msec interval */
  board_gpio_write(pin, LOW);
}

static void led_task(void)
{
  int led = LED0;

  /* LED0 -> LED1 -> LED2 -> LED3 -> LED0 -> ... (50msec interval) */

  while (1)
    {
      led_main(led);

      led = (led + 1)%4;
    }

  return;
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * Name: led_start()
 *
 * Description:
 *   Start LED flickering.
 *
 * Returned Value:
 *   Zero (OK) on success; Negative value on error.
 *
 * Assumptions/Limitations:
 *   none.
 *
 ****************************************************************************/

int led(void)
{
  int ret;

  ret = task_create("led_task",
                    80,     /* task priority   */
                    2048,    /* task stack size */
                    (main_t)led_task,
                    NULL);
  if (ret < 0)
    {
      printf("Failed to create task for led. ret=%d\n", ret);
      return ret;
    }

  return OK;
}


