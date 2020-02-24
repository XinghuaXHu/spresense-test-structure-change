/***********************************************************************
 *
 *      File Name: baseband_stb.cpp
 *
 *      Description: Stub file of baseband_config.cpp
 *
 *      Notes: (C) Copyright 2016 Sony Corporation
 *
 *      Author: Hsingying Ho
 *
 ***********************************************************************
 */

#include "baseband_config.h"
#include <stdint.h>
#include "audio/high_level_api/as_high_level_api.h" // SDK/include/audio/high_level_api

/*--------------------------------------------------------------------*/
/* Common functions for both new and old high level APIs */
/*--------------------------------------------------------------------*/
int powerOff_Called = 0;
uint32_t powerOff_ReturnValue = AUDRLT_STATUSCHANGED;
/* 戻り値は以下の値をとる
 AUDRLT_STATUSCHANGED: 正常終了
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::powerOff()
{
	powerOff_Called++;
	return powerOff_ReturnValue;
}

/*--------------------------------------------------------------------*/
int setActiveBaseband_Called = 0;
uint32_t setActiveBaseband_ReturnValue = AUDRLT_STATUSCHANGED;
/* 戻り値は以下の値をとる
 AUDRLT_STATUSCHANGED: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(bbInitFlagが何もセットされていないとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::setActiveBaseband()
{
	setActiveBaseband_Called++;
	return setActiveBaseband_ReturnValue;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int initMicGain_Called = 0;
AudioCommand initMicGain_cmd;
uint32_t initMicGain_ReturnValue = AUDRLT_INITMICGAINCMPLT;
/* 戻り値は以下の値をとる
 AUDRLT_INITMICGAINCMPLT: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(SubCode該当なしのとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::initMicGain(AudioCommand &cmd)
{
	initMicGain_cmd = cmd;
	initMicGain_Called++;
	return initMicGain_ReturnValue;
}

/*--------------------------------------------------------------------*/
int initOutputSelect_Called = 0;
AudioCommand initOutputSelect_cmd;
uint32_t initOutputSelect_ReturnValue = AUDRLT_INITOUTPUTSELECTCMPLT;
/* 戻り値は以下の値をとる
 AUDRLT_INITOUTPUTSELECTCMPLT: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(SubCode該当なしのとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::initOutputSelect(AudioCommand &cmd)
{
	initOutputSelect_cmd = cmd;
	initOutputSelect_Called++;
	return initOutputSelect_ReturnValue;
}

/*--------------------------------------------------------------------*/
int initClearStereo_Called = 0;
AudioCommand initClearStereo_cmd;
uint32_t initClearStereo_ReturnValue = AUDRLT_INITCLEARSTEREOCMPLT;
/* 戻り値は以下の値をとる
 AUDRLT_INITCLEARSTEREOCMPLT: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(SubCode該当なしのとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::initClearStereo(AudioCommand &cmd)
{
	initClearStereo_cmd = cmd;
	initClearStereo_Called++;
	return initClearStereo_ReturnValue;
}

/*--------------------------------------------------------------------*/
int setVolume_Called = 0;
AudioCommand setVolume_cmd;
uint32_t setVolume_ReturnValue = AUDRLT_SETVOLUMECMPLT;
/* 戻り値は以下の値をとる
 AUDRLT_SETVOLUMECMPLT: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(SubCode該当なしのとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::setVolume(AudioCommand &cmd)
{
	setVolume_cmd = cmd;
	setVolume_Called++;
	return setVolume_ReturnValue;
}

/*--------------------------------------------------------------------*/
int setVolumeMute_Called = 0;
AudioCommand setVolumeMute_cmd;
uint32_t setVolumeMute_ReturnValue = AUDRLT_SETVOLUMEMUTECMPLT;
/* 戻り値は以下の値をとる
 AUDRLT_SETVOLUMEMUTECMPLT: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(SubCode該当なしのとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::setVolumeMute(AudioCommand &cmd)
{
	setVolumeMute_cmd = cmd;
	setVolumeMute_Called++;
	return setVolumeMute_ReturnValue;
}

/*--------------------------------------------------------------------*/
int setBeep_Called = 0;
AudioCommand setBeep_cmd;
uint32_t setBeep_ReturnValue = AUDRLT_SETBEEPCMPLT;
/* 戻り値は以下の値をとる
 AUDRLT_SETBEEPCMPLT: 正常終了
 AUDRLT_ERRORRESPONSE: エラー(SubCode該当なしのとき)
 AUDRLT_ERRORATTENTION: エラー
*/
uint32_t BasebandConfig::setBeep(AudioCommand &cmd)
{
	setBeep_cmd = cmd;
	setBeep_Called++;
	return setBeep_ReturnValue;
}

/*--------------------------------------------------------------------*/
int clearBasebandInitConfig_Called = 0;
void BasebandConfig::clearBasebandInitConfig()
{
	clearBasebandInitConfig_Called++;
}

