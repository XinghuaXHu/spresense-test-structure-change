/****************************************************************************
 * demo/collet_box/tracker/include/tracker_tram_sensor_control.h
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Suzunosuke Hida <Suzunosuke.Hida@sony.com>
 *           Tomonobu Hayakawa <Tomonobu.Hayakawa@sony.com>
 *           Tetsuro Itabashi <Tetsuro.x.Itabashi@sony.com>
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

#ifndef __DEMO_COLLET_BOX_TRACKER_TRACKER_TRAM_SENSOR_CONTROL_H
#define __DEMO_COLLET_BOX_TRACKER_TRACKER_TRAM_SENSOR_CONTROL_H

/**
 * @defgroup tram state transition API 
 * @{
 */

/*--------------------------------------------------------------------*/
/*  Pre-processor Definitions                                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*  Public Structure                                                  */
/*--------------------------------------------------------------------*/
  
/*--------------------------------------------------------------------*/
/*  Public Types                                                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*  External Interface                                                */
/*--------------------------------------------------------------------*/

int TramOpenSensors(void);
int TramCloseSensors(void);
int TramStartSensors(void);
int TramStopSensors(void);
int TramSendMathFuncEvent(void);
int TramChangeScuSettings(void);

/**
 * @}
 */

#endif /* __DEMO_COLLET_BOX_TRACKER_TRACKER_TRAM_SENSOR_CONTROL_H */

