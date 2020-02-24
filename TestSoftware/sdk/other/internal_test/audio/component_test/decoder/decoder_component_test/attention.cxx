/****************************************************************************
 * test/internal_test/audio/component_test/decoder/decoder_component_test/
 * attention.cpp
 *
 *   Copyright (C) 2017 Sony Corporation
 *   Author: Suzunosuke Hida <Suzunosuke.Hida@sony.com>
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

#include <stdio.h>

extern "C" {

#ifdef ATTENTION_USE_FILENAME_LINE

void _Attention(uint8_t module_id, uint8_t attention_id, uint8_t sub_code,
                const char* file_name, uint32_t line)
{
  printf("_Attention:module_id=%d, attention_id=%d, sub_code=%d, file=%s"
         " LINE=%d\n",
         module_id, attention_id, sub_code, file_name, line);
}

#else /* ATTENTION_USE_FILENAME_LINE */

void _Attention(uint8_t module_id, uint8_t attention_id, uint8_t sub_code)
{
  printf("_Attention:module_id=%d, attention_id=%d, sub_code=%d\n",
         module_id, attention_id, sub_code);
}

#endif /* ATTENTION_USE_FILENAME_LINE */

} /* extern "C" */
