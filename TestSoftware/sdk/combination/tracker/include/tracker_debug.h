/****************************************************************************
 * demo/collet_box/tracker/tracker_debug.h
 *
 *   Copyright (C) 2017 Sony Corporation. All rights reserved.
 *   Author: Yutaka Miyajima <Yutaka.Miyajima@sony.com>
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

#ifndef __DEMO_COLLET_BOX_TRACKER_TRACKER_DEBUG_H
#define __DEMO_COLLET_BOX_TRACKER_TRACKER_DEBUG_H

#ifdef CONFIG_DEMO_COLLET_TRACKER_DEBUG_ERROR
  #define tracker_debug_error(format, ...) \
  printf(format, ##__VA_ARGS__)
#else
  #define tracker_debug_error(x...)
#endif

#ifdef CONFIG_DEMO_COLLET_TRACKER_DEBUG_WARN
  #define tracker_debug_warn(format, ...) \
  printf(format, ##__VA_ARGS__)
#else
  #define tracker_debug_warn(x...)
#endif

#ifdef CONFIG_DEMO_COLLET_TRACKER_DEBUG_INFO
  #define tracker_debug_info(format, ...) \
  printf(format, ##__VA_ARGS__)
#else
  #define tracker_debug_info(x...)
#endif

#endif /* __DEMO_COLLET_BOX_TRACKER_TRACKER_DEBUG_H */
