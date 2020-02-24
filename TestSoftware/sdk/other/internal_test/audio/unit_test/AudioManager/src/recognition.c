/*
Voice Recognition Objectのstub
本物とcallback関数ポインタの型が違う
*/
#include "voice_recognition_object.h" // stub version
#include <stdlib.h>
#include "stub/recognition.h"

/* VADのデフォルトパラメータ */
uint32_t VADCoef_table[16] = {
	0x00010007, 0x13330b24, 0x0000030a, 0x73200000,
	0x2800645c, 0x0b245500, 0x030a1333, 0x00000000,
	0x645c7320, 0x55002800, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

/* AS_voiceRecognitionObjectSetWaitKeyStatus */
int AS_voiceRecognitionObjectSetWaitKeyStatus_Called = 0;
void *AS_voiceRecognitionObjectSetWaitKeyStatus_cbFunc = NULL;
uint32_t AS_voiceRecognitionObjectSetWaitKeyStatus_mathfuncConfigTableAddress = NULL;
err_t AS_voiceRecognitionObjectSetWaitKeyStatus_ReturnValue = ERR_OK;
uint8_t AS_voiceRecognitionObjectStartVoiceCommand_vadParamTableAddress = NULL;
err_t AS_voiceRecognitionObjectSetWaitKeyStatus(voiceTriggerCB cbFunc, uint32_t mathfuncConfigTableAddress,uint32_t vadParamTableAddress)
{
	AS_voiceRecognitionObjectSetWaitKeyStatus_Called++;
	AS_voiceRecognitionObjectSetWaitKeyStatus_cbFunc = (void *)cbFunc;
	AS_voiceRecognitionObjectSetWaitKeyStatus_mathfuncConfigTableAddress = mathfuncConfigTableAddress;
	AS_voiceRecognitionObjectStartVoiceCommand_vadParamTableAddress = vadParamTableAddress;
	return AS_voiceRecognitionObjectSetWaitKeyStatus_ReturnValue;
}

/* AS_voiceRecognitionObjectSetReadyStatus */
int AS_voiceRecognitionObjectSetReadyStatus_Called = 0;
err_t AS_voiceRecognitionObjectSetReadyStatus_ReturnValue = ERR_OK;
err_t AS_voiceRecognitionObjectSetReadyStatus(void)
{
	AS_voiceRecognitionObjectSetReadyStatus_Called++;
	return AS_voiceRecognitionObjectSetReadyStatus_ReturnValue;
}

/* AS_startVoiceCmd */
int AS_voiceRecognitionObjectStartVoiceCommand_Called = 0;
uint16_t AS_voiceRecognitionObjectStartVoiceCommand_KeyWord = 0;
uint8_t AS_voiceRecognitionObjectStartVoiceCommand_vad_only = 0;
void *AS_voiceRecognitionObjectStartVoiceCommand_cbFunc = NULL;
uint8_t *AS_voiceRecognitionObjectStartVoiceCommand_vad_param = NULL;
err_t AS_voiceRecognitionObjectStartVoiceCommand_ReturnValue = ERR_OK;
err_t AS_startVoiceCmd(uint16_t KeyWord, uint8_t vad_only, voiceCommandCB cbFunc, uint8_t *p_vad_param)
{
	AS_voiceRecognitionObjectStartVoiceCommand_Called++;
	AS_voiceRecognitionObjectStartVoiceCommand_KeyWord = KeyWord;
	AS_voiceRecognitionObjectStartVoiceCommand_vad_only = vad_only;
	AS_voiceRecognitionObjectStartVoiceCommand_cbFunc = (void *)cbFunc;
	AS_voiceRecognitionObjectStartVoiceCommand_vad_param = p_vad_param;
	return AS_voiceRecognitionObjectStartVoiceCommand_ReturnValue;
}

/* AS_stopVoiceCmd */
int AS_voiceRecognitionObjectStopVoiceCommand_Called = 0;
err_t AS_voiceRecognitionObjectStopVoiceCommand_ReturnValue = ERR_OK;
err_t AS_stopVoiceCmd(void)
{
	AS_voiceRecognitionObjectStopVoiceCommand_Called++;
	return AS_voiceRecognitionObjectStopVoiceCommand_ReturnValue;
}

/* AS_activateVoiceCmd */
int AS_activateVoiceCmd_Called = 0;
bool AS_activateVoiceCmd_Called_ReturnValue = true;
bool AS_activateVoiceCmd(void)
{
	AS_activateVoiceCmd_Called++;
	return AS_activateVoiceCmd_Called_ReturnValue;
}

/* AS_deactivateVoiceCmd */
int AS_deactivateVoiceCmd_Called = 0;
bool AS_deactivateVoiceCmd_ReturnValue = true;
bool AS_deactivateVoiceCmd(void)
{
	AS_deactivateVoiceCmd_Called++;
	return AS_deactivateVoiceCmd_ReturnValue;
}
