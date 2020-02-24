/****************************************************************************
 * audio_recog2/worker_recognizer_load/userproc/src/rcgproc.cpp
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
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

#include "rcgproc.h"
#include "shm_def.h"

volatile char *buf;
static int i_cnt =0;
static int e_cnt =0;
static int e_cnt2 =0;
static int f_cnt =0;
static int s_cnt =0;

/*************************************************************
  private function
*************************************************************/

void RcgProc::pr_shm_on_init(int cnt)
{

  buf = (volatile char *)TARGET_ADDR;
  
  buf[RCG_I_ADDR|RCG_OFFS0] = (char)(RCG1_I_KEY);
  buf[RCG_I_ADDR|RCG_OFFS1] = (char)(cnt>>8);
  buf[RCG_I_ADDR|RCG_OFFS2] = (char)(cnt);
  buf[RCG_I_ADDR|RCG_OFFS3] = (char)RCG_INFO0;

}
 
void RcgProc::pr_shm_on_exec1(int cnt)
{

  buf = (volatile char *)TARGET_ADDR;
  
  buf[RCG_E_ADDR|RCG_OFFS0] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR|RCG_OFFS1] = (char)(cnt>>8);
  buf[RCG_E_ADDR|RCG_OFFS2] = (char)(cnt);
  buf[RCG_E_ADDR|RCG_OFFS3] = (char)RCG_INFO0;
      
  buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*1)] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR|RCG_OFFS1|(FLD_OFFS*1)] = (char)(offs>>8);
  buf[RCG_E_ADDR|RCG_OFFS2|(FLD_OFFS*1)] = (char)(offs);
  buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*1)] = (char)RCG_INFO1;
  
  buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*2)] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR|RCG_OFFS1|(FLD_OFFS*2)] = (char)(val1>>8);
  buf[RCG_E_ADDR|RCG_OFFS2|(FLD_OFFS*2)] = (char)(val1);
  buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*2)] = (char)RCG_INFO2;
      
  buf[RCG_E_ADDR|RCG_OFFS0|(FLD_OFFS*3)] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR|RCG_OFFS1|(FLD_OFFS*3)] = (char)(val2>>8);
  buf[RCG_E_ADDR|RCG_OFFS2|(FLD_OFFS*3)] = (char)(val2);
  buf[RCG_E_ADDR|RCG_OFFS3|(FLD_OFFS*3)] = (char)RCG_INFO3;
}

void RcgProc::pr_shm_on_exec2(int cnt)
{

  buf = (volatile char *)TARGET_ADDR;
  
  buf[RCG_E_ADDR2|RCG_OFFS0] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR2|RCG_OFFS1] = (char)(cnt>>8);
  buf[RCG_E_ADDR2|RCG_OFFS2] = (char)(cnt);
  buf[RCG_E_ADDR2|RCG_OFFS3] = (char)RCG_INFO0;
      
  buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*1)] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*1)] = (char)(offs>>8);
  buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*1)] = (char)(offs);
  buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*1)] = (char)RCG_INFO1;
  
  buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*2)] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*2)] = (char)(val1>>8);
  buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*2)] = (char)(val1);
  buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*2)] = (char)RCG_INFO2;
      
  buf[RCG_E_ADDR2|RCG_OFFS0|(FLD_OFFS*3)] = (char)(RCG1_E_KEY);
  buf[RCG_E_ADDR2|RCG_OFFS1|(FLD_OFFS*3)] = (char)(val2>>8);
  buf[RCG_E_ADDR2|RCG_OFFS2|(FLD_OFFS*3)] = (char)(val2);
  buf[RCG_E_ADDR2|RCG_OFFS3|(FLD_OFFS*3)] = (char)RCG_INFO3;
}
void RcgProc::pr_shm_on_flush(int cnt)
{

  buf = (volatile char *)TARGET_ADDR;
  
  buf[RCG_F_ADDR|RCG_OFFS0] = (char)(RCG1_F_KEY);
  buf[RCG_F_ADDR|RCG_OFFS1] = (char)(cnt>>8);
  buf[RCG_F_ADDR|RCG_OFFS2] = (char)(cnt);
  buf[RCG_F_ADDR|RCG_OFFS3] = (char)RCG_INFO0;

}

void RcgProc::pr_shm_on_set(int cnt, uint16_t d1, uint16_t d2, uint16_t d3)
{
  volatile int32_t data;
  long int i=0;

  buf = (volatile char *)TARGET_ADDR;
  
  buf[RCG_S_ADDR|RCG_OFFS0] = (char)(RCG1_S_KEY);
  buf[RCG_S_ADDR|RCG_OFFS1] = (char)(cnt>>8);
  buf[RCG_S_ADDR|RCG_OFFS2] = (char)(cnt);
  buf[RCG_S_ADDR|RCG_OFFS3] = (char)RCG_INFO0;

  buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*1)] = (char)(RCG1_S_KEY);
  buf[RCG_S_ADDR|RCG_OFFS1|(FLD_OFFS*1)] = (char)(d1>>8);
  buf[RCG_S_ADDR|RCG_OFFS2|(FLD_OFFS*1)] = (char)(d1);
  buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*1)] = (char)RCG_INFO1;
  
  buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*2)] = (char)(RCG1_S_KEY);
  buf[RCG_S_ADDR|RCG_OFFS1|(FLD_OFFS*2)] = (char)(d2>>8);
  buf[RCG_S_ADDR|RCG_OFFS2|(FLD_OFFS*2)] = (char)(d2);
  buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*2)] = (char)RCG_INFO2;
      
  buf[RCG_S_ADDR|RCG_OFFS0|(FLD_OFFS*3)] = (char)(RCG1_S_KEY);
  buf[RCG_S_ADDR|RCG_OFFS1|(FLD_OFFS*3)] = (char)(d3>>8);
  buf[RCG_S_ADDR|RCG_OFFS2|(FLD_OFFS*3)] = (char)(d3);
  buf[RCG_S_ADDR|RCG_OFFS3|(FLD_OFFS*3)] = (char)RCG_INFO3;

  if(d1 == SP_INST)
    {
       for(i=0; i<d2; i++)
         {
           data = (int32_t)i;  
         }
    
    }
  else if(d1 == SP_INST2)
    {
       while(1);
    } 
}
/*--------------------------------------------------------------------*/
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void RcgProc::init(InitRcgParam *param)
{
  /* Init recognition process. */

  m_ch_num       = param->ch_num;
  m_sample_width = param->sample_width;

  offs = 0;
  val1 = 1;
  val2 = 1;
  
  param->result.result_code = CustomprocCommand::ExecOk;

  pr_shm_on_init(i_cnt++);
}

/*--------------------------------------------------------------------*/
void RcgProc::exec(ExecRcgParam *param)
{
  /* Execute recognition process to input audio data. */

  /* Here, in this example, simply return max, min, average
   * and sample num of each audio frames to application.
   */

  if (!m_enable)
    {
      /* If not enabled, do not inform result to application. */

      param->exec_cmd.output.size = 0;
      param->result.inform_req  = 0;
      param->result.result_code = CustomprocCommand::ExecOk;

      pr_shm_on_exec2(e_cnt2++);

      return;
    }

  int16_t *data = (int16_t *)param->exec_cmd.input.addr;
  int16_t max = 0x8000;
  int16_t min = 0x7fff;
  int16_t smp = param->exec_cmd.input.size / m_ch_num / m_sample_width;
  int16_t avg = 0;
  int32_t sum = 0;

  for (uint32_t cnt = 0; cnt < param->exec_cmd.input.size; cnt += (m_ch_num * m_sample_width))
    {
      max = (*data > max) ? *data : max;
      min = (min > *data) ? *data : min;
      sum += *data;

      /* Next sample */

      data += m_ch_num;
    }

  avg = (int16_t)(sum / smp);

  /* Set output data and size. */

  param->exec_cmd.output.size = 8;
  
  int16_t *output = (int16_t *)param->exec_cmd.output.addr;

#if 0
  output[0] = max;
  output[1] = min;
  output[2] = avg;
  output[3] = smp;
#else
  output[0] = max + T_OFFSET;
  output[1] = min + T_OFFSET;
  output[2] = avg;
  output[3] = smp;
#endif
  /* Set inform request. */

  /* In this example, inform output to application every 100 frames.
   * This parameter can use for notify change point of recognition too.
   */

  static int s_inform_cnt = 0;

  param->result.inform_req  = ((s_inform_cnt++ % 100) == 0) ? 1 : 0;

  /* Set result code. */

  param->result.result_code = CustomprocCommand::ExecOk;
  
  pr_shm_on_exec1(e_cnt++);
}

/*--------------------------------------------------------------------*/
void RcgProc::flush(FlushRcgParam *param)
{
  /* Flush signal process. */

  param->flush_cmd.output.size = 0;

  param->result.result_code = CustomprocCommand::ExecOk;
  
  buf = (volatile char *)TARGET_ADDR;

  pr_shm_on_flush(f_cnt++);
}

/*--------------------------------------------------------------------*/
void RcgProc::set(SetRcgParam *param)
{
  /* Set recognition process parameters. */

  /* In this examples, set Enable/Disable.
   */

  m_enable = param->enable;
  offs     = (uint16_t)param->offset;
  val1     = (uint16_t)param->value1;
  val2     = (uint16_t)param->value2;

  param->result.result_code = CustomprocCommand::ExecOk;
 
  pr_shm_on_set(s_cnt++, (uint16_t)param->offset, (uint16_t)param->value1, (uint16_t)param->value2);
}

