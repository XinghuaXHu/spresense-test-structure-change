/****************************************************************************
 * demo/collet_box/tracker/tracker_tram_sensor_control.cxx
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <memutils/s_stl/queue.h>

#include "sensing/sensor_command.h"
#include "sensing/sensor_id.h"
#include "memutils/memory_manager/MemHandle.h"

#include "tracker_debug.h"
#include "tracker_tram_accel_sensor.h"
#include "tracker_tram_magnetometer_sensor.h"
#include "tracker_tram_pressure_sensor.h"
#include "tracker_tram_temperature_sensor.h"
#include "sensing/barometer.h"
#include "sensing/transport_mode.h"

#include "tracker_tram_sensor_control.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* sensor instances */

static TramClass* s_tram_ins = NULL;
static BarometerClass* s_bar_ins = NULL;
static AccelSensor *p_accel_sensor = NULL;
static MagnetmeterSensor *p_mag_sensor = NULL;
static PressureSensor *p_press_sensor = NULL;
static TemperatureSensor *p_temp_sensor = NULL;

/****************************************************************************
 * Callback Function
 ****************************************************************************/

static int accel_read_callback(uint32_t context, AccelEvent ev_type, MemMgrLite::MemHandle &mh)
{
  if (ACCEL_EV_WM == ev_type)
    {
      /* get timestamp in millisecond */
      
      struct scutimestamp_s wm_ts = p_accel_sensor->wm_ts;
      uint32_t timestamp = 1000 * wm_ts.sec + ((1000 * wm_ts.tick) >> 15); /* tick in 32768 Hz */
      
      sensor_command_data_mh_t packet;
      packet.header.size = 0;
      packet.header.code = SendData;
      packet.self        = accelID;
      packet.time        = timestamp;
      packet.fs          = ACCEL_SAMPLING_FREQUENCY_CMD;
      packet.size        = ACCEL_WATERMARK_NUM;
      packet.mh          = mh;

      SF_SendSensorDataMH(&packet);
    }
  else if (ACCEL_EV_MF == ev_type)
    {
      sensor_command_data_t packet;
      packet.header.size = 0;
      packet.header.code = SendData;
      packet.self        = accelID;
      packet.time        = 0;
      packet.fs          = 0;
      packet.size        = 0;
      packet.is_ptr      = false;
      packet.data        = 1;

      SF_SendSensorData(&packet);
    }
  return 0;
}

/*------------------------------------------------------------*/
static int mag_read_callback(uint32_t context, MemMgrLite::MemHandle &mh)
{
  /* get timestamp in millisecond */
      
  struct scutimestamp_s wm_ts = p_mag_sensor->wm_ts;
  uint32_t timestamp = 1000 * wm_ts.sec + ((1000 * wm_ts.tick) >> 15); /* tick in 32768 Hz */

  sensor_command_data_mh_t packet;
  packet.header.size = 0;
  packet.header.code = SendData;
  packet.self        = magID;
  packet.time        = timestamp;
  packet.fs          = MAG_SAMPLING_FREQUENCY;
  packet.size        = MAG_WATERMARK_NUM;
  packet.mh          = mh;

  SF_SendSensorDataMH(&packet);

  return 0;
}

/*------------------------------------------------------------*/
static int press_read_callback(uint32_t context, MemMgrLite::MemHandle &mh)
{
  /* get timestamp in millisecond */
      
  struct scutimestamp_s wm_ts = p_press_sensor->wm_ts;
  uint32_t timestamp = 1000 * wm_ts.sec + ((1000 * wm_ts.tick) >> 15); /* tick in 32768 Hz */

  sensor_command_data_mh_t packet;
  packet.header.size = 0;
  packet.header.code = SendData;
  packet.self        = pressureID;
  packet.time        = timestamp;
  packet.fs          = PRESSURE_SAMPLING_FREQUENCY;
  packet.size        = PRESSURE_WATERMARK_NUM;
  packet.mh          = mh;

  SF_SendSensorDataMH(&packet);

  return 0;
}

/*------------------------------------------------------------*/
static int temp_read_callback(uint32_t context, MemMgrLite::MemHandle &mh)
{
  /* get timestamp in millisecond */
      
  struct scutimestamp_s wm_ts = p_temp_sensor->wm_ts;
  uint32_t timestamp = 1000 * wm_ts.sec + ((1000 * wm_ts.tick) >> 15); // tick in 32768 Hz

  sensor_command_data_mh_t packet;
  packet.header.size = 0;
  packet.header.code = SendData;
  packet.self        = tempID;
  packet.time        = timestamp;
  packet.fs          = TEMPERATURE_SAMPLING_FREQUENCY;
  packet.size        = TEMPERATURE_WATERMARK_NUM;
  packet.mh          = mh;

  SF_SendSensorDataMH(&packet);

  return 0;
}

/*------------------------------------------------------------*/
static bool accel_power_ctrl_callback(bool data)
{
  tracker_debug_info("power state: %d\n", data);

  if (data)
    {
      ScuSettings* settings = TramGetAccelScuSettings(s_tram_ins);
      DEBUGASSERT(settings);
      AccelSensorStartSensing(p_accel_sensor, settings);
    }
  else
    {
      AccelSensorStopSensing(p_accel_sensor);
    }

  return true;
}

/*------------------------------------------------------------*/
static bool magne_power_ctrl_callback(bool data)
{
  tracker_debug_info("power state: %d\n", data);

  if (data)
    {
      MagnetmeterSensorStartSensing(p_mag_sensor);
    }
  else
    {
      MagnetmeterSensorStopSensing(p_mag_sensor);
    }

  return true;
}

/*------------------------------------------------------------*/
static bool pressure_power_ctrl_callback(bool data)
{
  tracker_debug_info("power state: %d\n", data);

  if (data)
    {
      PressureSensorStartSensing(p_press_sensor);
    }
  else
    {
      PressureSensorStopSensing(p_press_sensor);
    }

  return true;
}

/*------------------------------------------------------------*/
static bool temperature_power_ctrl_callback(bool data)
{
  tracker_debug_info("power state: %d\n", data);

  if (data)
    {
      TemperatureSensorStartSensing(p_temp_sensor);
    }
  else
    {
      TemperatureSensorStopSensing(p_temp_sensor);
    }

  return true;
}

/*------------------------------------------------------------*/
static bool bar_receive_data(sensor_command_data_mh_t& data)
{
  tracker_debug_info("data.self: %d\n", data.self);
  BarometerWrite(s_bar_ins, &data);

  return true;
}

/*------------------------------------------------------------*/
static bool baro_power_ctrl_callback(bool data)
{
  tracker_debug_info("power state: %d\n", data);

  if (data)
    {
      BarometerStart(s_bar_ins);
    }
  else
    {
      BarometerStop(s_bar_ins);
    }

  return true;
}

/*------------------------------------------------------------*/
static bool tram_receive_event(sensor_command_data_t& data)
{
  tracker_debug_info("data.self: %d\n", data.self);

  /* ignore event from mathfunc... */

  return true;
}

/*------------------------------------------------------------*/
static bool tram_receive_data(sensor_command_data_mh_t& data)
{
  tracker_debug_info("data.self: %d\n", data.self);
  TramWrite(s_tram_ins, &data);

  return true;
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int init_physical_sensor(uint32_t context)
{
  int ret = 0;
  
  /* Accelerator */
  
  ret = AccelSensorCreate(&p_accel_sensor);
  if (ret < 0)
    {
      printf("AccelSensorCreate() failure. %d\n", ret);
      return -1;
    }
  
  ret = AccelSensorRegisterHandler(p_accel_sensor, accel_read_callback, context);
  if (ret < 0)
    {
      printf("AccelSensorRegisterHandler() failure. %d\n", ret);
      return -1;
    }
  
  /* Magnetmeter */
  
  ret = MagnetmeterSensorCreate(&p_mag_sensor);
  if (ret < 0)
    {
      printf("MagnetmeterSensorCreate() failure. %d\n", ret);
      return -1;
    }
  
  ret = MagnetmeterSensorRegisterHandler(p_mag_sensor, mag_read_callback, context);
  if (ret < 0)
    {
      printf("MagnetmeterSensorRegisterHandler() failure. %d\n", ret);
      return -1;
    }
  
  /* Pressure */
  
  ret = PressureSensorCreate(&p_press_sensor);
  if (ret < 0)
    {
      printf("BarometerSensorCreate() failure. %d\n", ret);
      return -1;
    }
  
  ret = PressureSensorRegisterHandler(p_press_sensor, press_read_callback, context);
  if (ret < 0)
    {
      printf("BarometerSensorRegisterHandler() failure. %d\n", ret);
      return -1;
    }
  
  /* Temperature */
  
  ret = TemperatureSensorCreate(&p_temp_sensor);
  if (ret < 0)
    {
      printf("TemperatureSensorCreate() failure. %d\n", ret);
      return -1;
    }
  
  ret = TemperatureSensorRegisterHandler(p_temp_sensor, temp_read_callback, context);
  if (ret < 0)
    {
      printf("TemperatureSensorRegisterHandler() failure. %d\n", ret);
      return -1;
    }
  return 0;
}

/*------------------------------------------------------------*/
static void finish_physical_sensor(void)
{
  AccelSensorDestroy(p_accel_sensor);
  MagnetmeterSensorDestroy(p_mag_sensor);
  PressureSensorDestroy(p_press_sensor);
  TemperatureSensorDestroy(p_temp_sensor);
}

/*------------------------------------------------------------*/
static int init_logical_sensor(void)
{
  int ret;
  
  /* Tram */

  s_tram_ins = TramCreate(SENSOR_DSP_CMD_BUF_POOL);
  ret = TramOpen(s_tram_ins);
  if (ret < 0)
    {
      printf("TramOpen() failure. %d\n", ret);
      return -1;
    }

  /* Barometer */
  
  s_bar_ins = BarometerCreate();
  ret = BarometerOpen(s_bar_ins);
  if (ret < 0)
    {
      printf("BarometerOpen() failure. %d\n", ret);
      return -1;
    }

  /* set barometer as owner of pressure and temperature sensor */
  
  p_press_sensor->owner = s_bar_ins;
  p_temp_sensor->owner = s_bar_ins;

  return 0;
}

/*------------------------------------------------------------*/
static void finish_logical_sensor(void)
{
  TramClose(s_tram_ins);
  BarometerClose(s_bar_ins);
}

/*------------------------------------------------------------*/
static void regist_physical_sensor(void)
{
  sensor_command_register_t reg;

  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = accelID;
  reg.subscriptions     = 0; 
  reg.callback          = NULL;
  reg.callback_mh       = NULL;
  reg.callback_pw       = accel_power_ctrl_callback;
  SF_SendSensorResister(&reg);

  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = magID;
  reg.subscriptions     = 0; 
  reg.callback          = NULL;
  reg.callback_mh       = NULL;
  reg.callback_pw       = magne_power_ctrl_callback;
  SF_SendSensorResister(&reg);
  
  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = pressureID;
  reg.subscriptions     = 0; 
  reg.callback          = NULL;
  reg.callback_mh       = NULL;
  reg.callback_pw       = pressure_power_ctrl_callback;
  SF_SendSensorResister(&reg);
  
  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = tempID;
  reg.subscriptions     = 0; 
  reg.callback          = NULL;
  reg.callback_mh       = NULL;
  reg.callback_pw       = temperature_power_ctrl_callback;
  SF_SendSensorResister(&reg);
}

/*------------------------------------------------------------*/
static void release_physical_sensor(void)
{
  sensor_command_release_t rel;

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = accelID;
  SF_SendSensorRelease(&rel);

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = magID;
  SF_SendSensorRelease(&rel);

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = pressureID;
  SF_SendSensorRelease(&rel);
  
  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = tempID;
  SF_SendSensorRelease(&rel);
}

/*------------------------------------------------------------*/
static void regist_logical_sensor(void)
{
  sensor_command_register_t reg;

  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = barometerID;
  reg.subscriptions     = (0x01 << pressureID) | (0x01 << tempID);
  reg.callback          = NULL;
  reg.callback_mh       = bar_receive_data;
  reg.callback_pw       = baro_power_ctrl_callback;
  SF_SendSensorResister(&reg);

  reg.header.size       = 0;
  reg.header.code       = ResisterClient;
  reg.self              = tramID;
  reg.subscriptions     = (0x01 << accelID) | (0x01 << magID) | (0x01 << barometerID);
  reg.callback          = tram_receive_event;
  reg.callback_mh       = tram_receive_data;
  reg.callback_pw       = NULL;
  SF_SendSensorResister(&reg);
}

/*------------------------------------------------------------*/
static void release_logical_sensor(void)
{
  sensor_command_release_t rel;

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = tramID;
  SF_SendSensorRelease(&rel);

  rel.header.size = 0;
  rel.header.code = ReleaseClient;
  rel.self        = barometerID;
  SF_SendSensorRelease(&rel);
}

/****************************************************************************
 * External Interface
 ****************************************************************************/
int TramOpenSensors(void)
{
  /* initialize physical sensor */

  init_physical_sensor(0);
  regist_physical_sensor();

  /* initialize logical sensor */

  init_logical_sensor();
  regist_logical_sensor();
  
  return 0;
}

/*------------------------------------------------------------*/
int TramCloseSensors(void)
{
  /* stop sensors first */
  
  AccelSensorStopSensing(p_accel_sensor);
  MagnetmeterSensorStopSensing(p_mag_sensor);
  PressureSensorStopSensing(p_press_sensor);
  TemperatureSensorStopSensing(p_temp_sensor);

  /* finalize logical sensors */

  release_logical_sensor();
  finish_logical_sensor();
  
  /* finalize physical sensors */

  release_physical_sensor();
  finish_physical_sensor();
  
  return 0;
}

/*------------------------------------------------------------*/
int TramStartSensors(void)
{
  return TramStart(s_tram_ins);
}

/*------------------------------------------------------------*/
int TramStopSensors(void)
{
  return TramStop(s_tram_ins);
}

/*------------------------------------------------------------*/
int TramSendMathFuncEvent(void)
{
  return TramHandleEvent(s_tram_ins, MathFuncEvent);
}

/*------------------------------------------------------------*/
int TramChangeScuSettings(void)
{
  ScuSettings* settings = TramGetAccelScuSettings(s_tram_ins);
  DEBUGASSERT(settings);

  return AccelSensorChangeScuSetting(p_accel_sensor, settings);
}
